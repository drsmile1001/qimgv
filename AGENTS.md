# PROJECT KNOWLEDGE BASE

**生成時間：** 2026-03-19
**Commit：** 5e919674
**Branch：** master

## OVERVIEW

qimgv 是一個 C++17/Qt 圖片檢視器（版本 1.0.2），支援靜態圖、動畫、可選的 libmpv 影片播放，以及 OpenCV 高品質縮放。

## STRUCTURE

```
qimgv/                    # 根目錄
├── CMakeLists.txt         # 根建置腳本（選項：VIDEO_SUPPORT, OPENCV_SUPPORT, EXIV2, KDE_SUPPORT）
├── qimgv/                 # 主應用源碼
│   ├── core.cpp/h         # 應用核心，連接所有元件
│   ├── settings.cpp/h     # 全域設定單例（枚舉定義也在此）
│   ├── main.cpp           # 入口點
│   ├── components/        # 後端子系統（載入、快取、縮放等）
│   ├── gui/               # UI 層（mainwindow, viewers, overlays 等）
│   ├── sourcecontainers/  # Image 資料模型（Image 抽象類別及子類）
│   ├── utils/             # 靜態工具類（ImageLib, FileOperations 等）
│   ├── 3rdparty/          # QtOpenCV（OpenCV/Qt 橋接）
│   └── res/               # 資源（圖示、翻譯、qrc）
├── plugins/player_mpv/    # MPV 播放器外掛（獨立 CMake 目標）
├── scripts/               # 建置/安裝腳本（Fedora 範例）
├── docs/                  # 開發文件
└── build[-novideo]/       # 建置輸出目錄（勿提交）
```

## WHERE TO LOOK

| 任務 | 位置 | 備註 |
|------|------|------|
| 新增使用者操作/快捷鍵 | `qimgv/components/actionmanager/` | 對應 `qimgv/utils/inputmap.h` |
| 修改影像顯示邏輯 | `qimgv/gui/viewers/imageviewerv2.cpp` | 繼承 QGraphicsView |
| 修改檔案載入流程 | `qimgv/components/loader/` + `directorymodel.cpp` | Loader 使用 QThreadPool |
| 新增影像操作（裁切、旋轉等） | `qimgv/utils/imagelib.h/cpp` + `core.cpp` 中的 slots | ImageLib 為靜態方法類 |
| UI overlay（浮動訊息、進度等） | `qimgv/gui/overlays/` | Proxy 模式：`*proxy.h` 包裝 actual widget |
| 縮圖生成 | `qimgv/components/thumbnailer/` + `cache/thumbnailcache.h` | 非同步 runnable 模式 |
| 設定選項 | `qimgv/settings.cpp` | QSettings 包裝，枚舉定義在 `settings.h` |
| 影片播放 | `plugins/player_mpv/` + `qimgv/gui/viewers/videoplayer.h` | 動態載入 .so/.dll |
| 跨平台檔案系統監控 | `qimgv/components/directorymanager/watchers/` | Linux/Windows 各自實作 |
| 主題/樣式 | `qimgv/themestore.cpp` + `qimgv/proxystyle.cpp` | |

## CODE MAP

| 類別 | 位置 | 角色 |
|------|------|------|
| `Core` | `qimgv/core.h` | 頂層協調者，連接 model/GUI/actions |
| `MainWindow (MW)` | `qimgv/gui/mainwindow.h` | 主視窗，持有所有 UI 子元件 |
| `DirectoryModel` | `qimgv/components/directorymodel.h` | 檔案列表 + 快取 + 載入器整合 |
| `DirectoryPresenter` | `qimgv/components/directorypresenter.h` | MVP presenter，連接 model 與 IDirectoryView |
| `DirectoryManager` | `qimgv/components/directorymanager/` | 檔案系統掃描與排序 |
| `Loader` | `qimgv/components/loader/loader.h` | 非同步影像載入（QThreadPool） |
| `Cache` | `qimgv/components/cache/cache.h` | 記憶體影像快取（path → Image） |
| `Scaler` | `qimgv/components/scaler/scaler.h` | 非同步縮放，支援 Qt 或 OpenCV 濾鏡 |
| `Thumbnailer` | `qimgv/components/thumbnailer/` | 非同步縮圖生成 |
| `Image` | `qimgv/sourcecontainers/image.h` | 抽象基類：`ImageStatic`, `ImageAnimated`, `Video` |
| `ImageViewerV2` | `qimgv/gui/viewers/imageviewerv2.h` | 主影像顯示（QGraphicsView） |
| `ViewerWidget` | `qimgv/gui/viewers/viewerwidget.h` | 包裝 ImageViewerV2 + VideoPlayer |
| `ActionManager` | `qimgv/components/actionmanager/` | 快捷鍵 → 動作映射 |
| `Settings` | `qimgv/settings.h` | 全域單例，QSettings 包裝 |
| `ImageLib` | `qimgv/utils/imagelib.h` | 靜態影像操作（旋轉、裁切、縮放） |
| `ImageFactory` | `qimgv/sourcecontainers/imagefactory.h` | 根據 `DocumentInfo` 建立 `ImageStatic`/`ImageAnimated`/`Video` |
| `DocumentInfo` | `qimgv/sourcecontainers/documentinfo.h` | 以 `QMimeDatabase` + `QImageReader` 偵測格式與動畫支援 |
| `VideoPlayerInitProxy` | `qimgv/gui/viewers/videoplayerinitproxy.h` | 影片外掛懶載入 proxy；實際播放器由 `QLibrary` 動態載入 |

## CONVENTIONS

- **標頭保護**：全部使用 `#pragma once`（非 `#ifndef` guard）
- **命名**：類別 PascalCase，檔案小寫（`imageviewerv2.cpp`），枚舉全大寫（`SORT_NAME`, `FIT_WINDOW`）
- **指標語意**：影像物件一律用 `std::shared_ptr<Image>`；可修改影像用 `std::unique_ptr<QImage>`
- **非同步模式**：耗時操作（載入/縮放/縮圖）均使用 QThreadPool + `*Runnable` 模式
- **Proxy 模式**：overlay 元件若需非同步初始化，提供 `*Proxy` 包裝類（如 `FloatingMessageProxy`）
- **Signal/Slot**：元件間通訊全用 Qt signal/slot，Core 負責主要連接（`connectComponents()`）
- **C++ 標準**：C++17（`target_compile_features(qimgv PRIVATE cxx_std_17)`）
- **枚舉定義**：放在 `settings.h` 中（`SortingMode`, `ImageFitMode`, `ScalingFilter` 等）
- **編譯選項宏**：`USE_EXIV2`, `USE_MPV`, `USE_OPENCV`, `USE_KDE_BLUR`

## ANTI-PATTERNS

- 勿在 UI 執行緒做影像解碼/縮放（必須透過 Loader/Scaler 的 QThreadPool）
- 勿直接持有 `Image*` 裸指標，必須用 `shared_ptr`
- 勿直接存取 `build/` 或 `build-novideo/` 目錄中的 autogen 代碼
- 勿在 `settings.h` 以外定義新的全域枚舉（應集中在此）
- 勿繞過 `DirectoryModel` 直接操作 `Cache` 或 `Loader`
- `Loader::loadAsyncPriority()` 內部呼叫 `clearPool()`，會取消所有預載任務——勿在不必要時呼叫此方法（`components/loader/loader.cpp`）
- `Cache` 無 LRU / 大小上限——大目錄可導致記憶體無限成長，新增快取操作時需注意（`components/cache/cache.cpp`）
- `ImageViewerV2::showImage()` 在設新 pixmap 前呼叫 `reset()`，可能造成黑屏閃爍——重構顯示流程前先確認此行為（`gui/viewers/imageviewerv2.cpp`）
- 無 CI/CD（無 `.github/workflows/`）——所有驗證需本機手動執行

## COMMANDS

```bash
# 依賴安裝（Fedora）
sudo dnf install -y qt5-qtbase-devel qt5-qtsvg-devel qt5-qttools-devel \
                   opencv-devel mpv-devel cmake ninja-build gcc-c++ git

# 建置（含影片支援）
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DOPENCV_SUPPORT=ON -DVIDEO_SUPPORT=ON
ninja -C build

# 建置（不含影片）
cmake -S . -B build-novideo -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DVIDEO_SUPPORT=OFF -DOPENCV_SUPPORT=ON
ninja -C build-novideo

# 安裝
sudo ninja -C build install

# 翻譯更新（需手動觸發）
ninja -C build qimgv_lupdate
```

## NOTES

- 兩個建置目錄並存：`build`（含影片）、`build-novideo`（無影片），`compile_commands.json` 符號連結指向 `build-novideo`
- Qt 版本可選：預設嘗試 Qt5，再 Qt6；Qt6 需額外連結 `OpenGLWidgets`
- GCC 最低版本 9.0（CMake 強制檢查）
- 翻譯檔在 `qimgv/res/translations/`，支援 zh_CN, uk_UA, es_ES, de_DE, fr_FR, tr_TR
- 影片外掛以動態庫形式載入（`player_mpv.so`），路徑由 `_QIMGV_PLUGIN_DIR` 宏定義
- macOS 支援存在但不完整（filesystem watcher 被注解掉）
- clangd index 快取在 `.cache/clangd/index/`
