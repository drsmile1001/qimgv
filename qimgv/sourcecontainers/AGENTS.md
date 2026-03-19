# sourcecontainers — 影像資料模型

Domain model 層：代表記憶體中的影像、縮圖、影片、檔案系統條目的資料容器。

## STRUCTURE

```
sourcecontainers/
├── image.h/cpp           # 抽象基類（Image）：定義 getPixmap/getImage/save 等介面
├── imagestatic.h/cpp     # 靜態圖實作（JPEG/PNG/BMP 等，透過 QImageReader）
├── imageanimated.h/cpp   # 動畫圖實作（GIF/APNG，使用 QMovie）
├── video.h/cpp           # 影片物件（僅持有路徑，由 VideoPlayer 播放）
├── thumbnail.h/cpp       # 縮圖資料容器
├── documentinfo.h/cpp    # 影像的後設資料（EXIF、檔案大小、解析度等）
└── fsentry.h/cpp         # 檔案系統條目（名稱、路徑、大小、時間、類型）
```

## CLASS HIERARCHY

```
Image（純虛）
├── ImageStatic      # QImageReader → QImage → QPixmap
├── ImageAnimated    # QMovie（逐幀）
└── Video            # 路徑容器，由 VideoPlayer 解碼
```

## POINTER SEMANTICS

| 情況 | 類型 |
|------|------|
| 傳遞/共享影像物件 | `std::shared_ptr<Image>` |
| 可修改的 QImage（編輯操作） | `std::unique_ptr<QImage>` |
| 唯讀的 QImage 參考 | `std::shared_ptr<const QImage>` |
| DocumentInfo | `std::unique_ptr<DocumentInfo>`（Image 持有） |

## CONVENTIONS

- `Image::load()` 是 protected 純虛方法，由子類實作，外部不直接呼叫
- EXIF 讀取透過 `DocumentInfo`（`USE_EXIV2` 宏控制）
- `isLoaded()` 旗標：`mLoaded = true` 代表資料已在記憶體中
- `isEdited()` 旗標：`mEdited = true` 代表有未儲存修改，影響 `saveConfirmOverlay`

## ANTI-PATTERNS

- 勿持有 `Image*` 裸指標，一律用 `shared_ptr<Image>`
- 勿在 UI 執行緒中呼叫 `Image::load()`（透過 `Loader` 的 QThreadPool）
- `Video` 物件不含像素資料，勿嘗試呼叫 `getPixmap()`/`getImage()` 進行渲染
