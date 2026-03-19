# qimgv 高速瀏覽照片效能分析與強化策略

## 使用情境

目標使用情境：高速瀏覽大型 JPEG 照片，例如相機連拍結果。

- 單張照片大小：6–10 MB
- 操作方式：方向鍵連續切換，快速比對前後張
- 效能要求：
  - 切換時畫面絕對不可出現黑畫面
  - 不可出現「先顯粗略畫面再換成清晰畫面」
  - 允許切換速度比解碼速度慢（新圖尚未準備好時，舊圖繼續撐在畫面上）
  - 允許進入資料夾時預先載入多張照片到記憶體

觀察到的現象：

- 3–4 MB JPEG：幾乎沒有效能問題，可快速連續切換
- 6 MB JPEG：快速按住方向鍵時，標題欄檔名快速跳動但畫面卡住，放開方向鍵後才載入停下的那張

---

## 現有架構設計

### 圖片解碼層

qimgv 本身不實作任何格式的低階解碼器。所有解碼透過 Qt 的 image IO 層發起：

- 靜態圖片：`QImageReader::read()`（`sourcecontainers/imagestatic.cpp`）
- 動畫圖片：`QMovie`、`QImage`、`QPixmap` constructors（`sourcecontainers/imageanimated.cpp`）

實際解碼由 Qt 內建 plugin 或系統安裝的第三方 Qt image plugin 執行。JPEG 解碼底層為 **libjpeg-turbo**（Qt 5.15+ 內建，版本 3.1.0），速度為原版 libjpeg 的 2–6 倍。qimgv 直接使用的第三方 library 只有：

- **Exiv2**：讀取 EXIF metadata（`documentinfo.cpp`，可選，CMake `EXIV2` 選項）
- **OpenCV**：高品質縮放，非解碼（`imagelib.cpp`，可選，CMake `OPENCV_SUPPORT` 選項）
- **mpv**（外部程式）：影片縮圖擷取

### 非同步載入機制

三個獨立的 `QThreadPool + QRunnable` 組合：

| 元件        | 路徑                      | Thread 數  | 用途             |
| ----------- | ------------------------- | ---------- | ---------------- |
| Loader      | `components/loader/`      | 2（硬編）  | 非同步解碼圖片   |
| Thumbnailer | `components/thumbnailer/` | 由設定控制 | 非同步產生縮圖   |
| Scaler      | `components/scaler/`      | 1（硬編）  | 非同步高品質縮放 |

### 預讀策略

切換圖片時，`Core::loadFileIndex()` 觸發以下流程（`core.cpp`，第 1282 行）：

```cpp
model->unloadExcept(entry.path, preload);    // 卸載非必要圖片
model->load(entry.path, async);              // 載入當前張
if(preload) {
    model->preload(model->nextOf(entry.path));  // 預讀下一張
    model->preload(model->prevOf(entry.path));  // 預讀上一張
}
```

預讀窗口固定為 **±1 張**，受 `settings->usePreloader()` 開關控制。

### 記憶體快取

- `components/cache/cache.cpp`：以 `QMap<QString, CacheItem*>` 儲存解碼完的 `Image` 物件
- 無 LRU、無固定上限；由上層在切圖時主動呼叫 `cache.trimTo()` 卸載
- `CacheItem` 以 `QSemaphore` 實作 lock，供 Scaler 等元件在使用期間防止被卸載

### 縮圖快取

- `components/cache/thumbnailcache.cpp`：磁碟快取，以 `md5(path + size + crop)` 為 key，存為 PNG
- 讀取時比對縮圖內嵌的 `lastModified` metadata，不符則視為過期並重新產生
- 產生縮圖時若 `QImageReader` 支援 scaled read（`setScaledSize`），優先使用以節省記憶體與時間

---

## 已確認的效能問題

### 黑畫面根本原因：`reset()` 在新圖就緒前清空畫面

`ImageViewerV2::showImage()`（`gui/viewers/imageviewerv2.cpp`，第 285 行）的第一行是 `reset()`：

```cpp
void ImageViewerV2::showImage(std::unique_ptr<QPixmap> _pixmap) {
    reset();   // 先清空 pixmapItem → 畫面立即變空白
    if(_pixmap) {
        pixmapItem.setPixmap(*pixmap);  // 才設定新圖
        // ...
    }
}
```

只要解碼時間 > 0，畫面必然出現一段空白期。

### 快速切換時預讀被自己打斷

`Loader::loadAsyncPriority()`（`components/loader/loader.cpp`，第 26 行）每次切換都清空整個 pool：

```cpp
void Loader::loadAsyncPriority(QString path) {
    clearPool();        // 清掉所有等待中的任務，包含預讀任務
    doLoadAsync(path, 1);
}
```

快速按住方向鍵時，每次切換都把前一次的解碼任務清掉，導致：

1. 所有中間幀都沒有機會解碼完成，cache 保持空白
2. 放開按鍵後只有最後一張從頭開始解碼
3. 預讀機制形同失效

### 預讀窗口 ±1 張在連拍場景不足

10 MB JPEG 解碼時間約 100–300ms，Loader 固定 2 threads，完成一輪預讀需 200–600ms。以每秒 3–5 張的快速切換速度，預讀必然追不上。

---

## 可行的強化策略

### A. 修正 `reset()` 順序（消除黑畫面，改動最小）

將 `showImage()` 改為「先設定新 pixmap，再清除舊狀態」，確保任何時刻畫面上都有圖：

```cpp
// 改法概念：不在新圖就緒前清空 pixmapItem
void ImageViewerV2::showImage(std::unique_ptr<QPixmap> _pixmap) {
    if(_pixmap) {
        stopPosAnimation();
        // 先清輔助元素，不清主要 pixmapItem
        pixmapItemScaled.hide();
        pixmapItemScaled.setPixmap(QPixmap());
        pixmapScaled.reset(nullptr);
        stopAnimation();
        movie = nullptr;

        pixmap = std::move(_pixmap);
        pixmapItem.setPixmap(*pixmap);  // 新圖直接替換，無空白中間狀態
        // ...其餘設定
    }
}
```

預期效果：切換時舊圖一直撐著，直到新圖完全解碼完才換掉，完全消除黑畫面。

### B. 拿掉快速切換時的 `clearPool()`（讓預讀實際發揮作用）

讓切換時僅更新 `state.currentFilePath` 目標，中間已在解碼的任務繼續跑完並進入 cache。`onModelItemReady` 已有路徑比對，只有最終停下的那張會觸發顯示：

```cpp
void Core::onModelItemReady(std::shared_ptr<Image> img, const QString &path) {
    if(path == state.currentFilePath) {
        guiSetImage(img);  // 只顯示最終目標張
    }
    // 中間幀進 cache，往回切時立即命中
}
```

改法：`loadAsyncPriority` 改為只調高優先級，不清空整個 pool；或對「已在執行中」的任務不中斷。

預期效果：快速切過的中間幀解碼後進入 cache，使用者往回比對時為即時顯示。

### C. 擴大預讀窗口（改善連拍瀏覽流暢度）

將預讀範圍從 ±1 擴大為可設定的 ±N，並加入方向性非對稱：

```cpp
// 往同一方向連續切換時，前方多預讀
int forward = isMovingForward ? preloadWindow : 1;
int backward = isMovingForward ? 1 : preloadWindow;
for(int i = 1; i <= forward; i++)
    model->preload(nthNextOf(entry.path, i));
for(int i = 1; i <= backward; i++)
    model->preload(nthPrevOf(entry.path, i));
```

| 情境               | 建議窗口                   |
| ------------------ | -------------------------- |
| 一般瀏覽           | ±1（現狀）                 |
| 連拍比對           | ±3–5                       |
| 進資料夾時全部預載 | 全部（可選，需記憶體確認） |

Loader thread 數也應改為可設定，連拍場景可提高到 4–6 threads。

### 策略優先順序

| 策略                   | 解決的問題                 | 改動範圍                 | 優先度 |
| ---------------------- | -------------------------- | ------------------------ | ------ |
| A. 修正 `reset()` 順序 | 黑畫面                     | `imageviewerv2.cpp` 一處 | 高     |
| B. 拿掉 `clearPool()`  | 預讀被打斷、中間幀無法快取 | `loader.cpp`、`core.cpp` | 高     |
| C. 擴大預讀窗口 ±N     | 連拍場景追不上切換速度     | `core.cpp`、設定         | 中     |

---

## 不採用的方向

- **低解析度先顯示（Low-Res First）**：先顯示模糊預覽再換成清晰版，會讓畫面在切換瞬間失去前一張的感覺，不符合使用情境需求
- **Progressive JPEG 解碼**：相機連拍通常輸出 Baseline JPEG，此方向無實際助益；且 Qt 的 `setScaledSize()` 對 Progressive JPEG 無法節省記憶體

---

## 相關原始碼位置

| 功能                       | 檔案                                                                |
| -------------------------- | ------------------------------------------------------------------- |
| 切圖主流程                 | `qimgv/core.cpp`，`loadFileIndex()`，第 1282 行                     |
| 顯示圖片（黑畫面根源）     | `qimgv/gui/viewers/imageviewerv2.cpp`，`showImage()`，第 285 行     |
| 非同步 Loader（clearPool） | `qimgv/components/loader/loader.cpp`                                |
| 預讀觸發                   | `qimgv/components/directorymodel.cpp`，`preload()`，第 335 行       |
| 記憶體快取                 | `qimgv/components/cache/cache.cpp`                                  |
| 靜態圖解碼                 | `qimgv/sourcecontainers/imagestatic.cpp`，`loadGeneric()`，第 31 行 |
| EXIF 讀取                  | `qimgv/sourcecontainers/documentinfo.cpp`，`loadExifTags()`         |
| OpenCV 縮放                | `qimgv/utils/imagelib.cpp`，`scaled_CV()`                           |

---

Qt 在 2026 年的地位
Qt 仍是桌面 native GUI 的最成熟選擇，但不再是「唯一合理選項」。Qt6 最大的結構性變化是 RHI（Rendering Hardware Interface）——統一 Vulkan/Metal/D3D12/OpenGL 後端，讓跨平台渲染有一致的效能基準。對圖片瀏覽器最直接的意義是 QPixmap 上傳 GPU 這條路徑在 Qt6 下已透過 RHI 優化，不再需要手工處理。
QGraphicsView 不是 legacy，但也不是 Qt 未來押注的方向。KDE 的 Gwenview 在 2021 年做了一個有趣的決定：反而把自製的雙緩衝邏輯刪掉，改用標準 QGraphicsPixmapItem，理由是「讓 Qt 自己負責優化」。這和 qimgv 的設計方向一致，代表這個技術選擇在同類軟體中是主流做法。

---

主要挑戰者
Rust + wgpu（最值得關注）
這是目前最有實質效能潛力的方向。Oculante (https://github.com/woelper/oculante)（1545 stars）、ViewSkater (https://viewskater.com/)（Iced + wgpu）是兩個活躍的案例。核心優勢是解碼後直接上傳 GPU texture，跳過 QImage → QPixmap → 渲染器 的中介轉換。zune-jpeg 這個 Rust 解碼器宣稱比 libjpeg-turbo 快 2–3x，但還沒有廣泛的第三方驗證。
GUI 框架層面，Iced 0.14（2025-12）引入 reactive rendering，CPU 消耗降低 60–80%，成熟度已到可考慮的程度。Slint 則以 C++/Rust 雙前端為賣點，但圖片解碼仍需手動整合。
Flutter Desktop
Impeller 引擎的 Linux/Windows Vulkan backend 截至 2026 年 3 月仍是 draft PR，Linux 桌面還跑在 Skia + ANGLE/OpenGL。對於圖片瀏覽器這種需要直接控制 decode 路徑的應用，Flutter 的封裝太不透明，不是合適選項。
Electron / Tauri
兩者都是 WebView 方案。Tauri 記憶體比 Electron 少 58%，包大小少 96%，但本質上無法做 GPU 直接 texture 上傳（除非用還不成熟的 WebGPU）。對 6–10MB JPEG 高速瀏覽場景不適用。
wxWidgets / FLTK
2026 年仍在活躍維護（wxWidgets 3.3.2 在 2026-03 發布），但效能和生態都不如 Qt，沒有轉移的動機。

---

GPU 直接上傳的實際情況
這是最容易被高估的方向。幾個關鍵事實：

- 主流 GPU 不支援 JPEG 硬解。NVDEC 只解影片格式，Intel Quick Sync 同樣。NVIDIA 的 nvJPEG 是 CUDA-based 的軟體方案，適合批次處理，不適合互動式切換。
- libjpeg-turbo 的 OpenCL 路徑已在 2024 年正式放棄，原因是 OpenCL overhead 比 JPEG 本身的計算量還大。
- QImage → QPixmap 的 CPU→GPU 上傳代價：4K 圖片約 10–20ms，6000×4000 的相機照片約 15–25ms。這個延遲比 JPEG 解碼時間（100–300ms）小一個數量級，不是主要瓶頸。
- Wuffs（Google 的安全解碼器）JPEG 效能落後於 libjpeg-turbo，不是替代選項。
  也就是說，花精力做 GPU 路徑最佳化，邊際效益遠小於修好 clearPool() 和 reset() 這兩個軟體設計問題。

---

對 qimgv 的實際意義
qimgv 的技術棧在 2026 年處於「合理但不前沿」的位置：

- Qt + libjpeg-turbo + QGraphicsView 組合在同類工具（gwenview、vimiv-qt 等）中是標準做法，沒有明顯的技術債
- 效能瓶頸不在框架選擇，而在前面討論的三個設計問題：reset() 順序、clearPool() 策略、預讀窗口大小
- 如果要追求極限效能，Rust + zune-jpeg + wgpu 的路線在理論上有優勢，但重寫成本高、GUI 框架成熟度仍不及 Qt，短期內不是合理投資
- Qt6 升級值得考慮，主要是 RHI 帶來的長期維護優勢，效能提升對現有瓶頸幫助有限
