# components — 後端子系統

所有非 UI 的核心邏輯，負責檔案系統、影像載入、快取、縮放、縮圖、腳本執行。

## STRUCTURE

```
components/
├── directorymodel.cpp/h      # 整合 DirectoryManager + Loader + Cache 的門面
├── directorypresenter.cpp/h  # MVP Presenter：連接 DirectoryModel ↔ IDirectoryView
├── actionmanager/            # 快捷鍵 → ActionID 映射管理
├── cache/                    # 記憶體影像快取（Cache + ThumbnailCache）
├── directorymanager/         # 檔案系統掃描、排序、跨平台 watcher
├── loader/                   # 非同步影像載入（QThreadPool + LoaderRunnable）
├── scaler/                   # 非同步影像縮放（QThreadPool + ScalerRunnable）
├── scriptmanager/            # 使用者自訂腳本管理
└── thumbnailer/              # 非同步縮圖生成（QThreadPool + ThumbnailerRunnable）
```

## WHERE TO LOOK

| 任務 | 位置 |
|------|------|
| 新增快捷鍵/動作 | `actionmanager/actionmanager.cpp` + `utils/inputmap.h` |
| 影像載入流程 | `loader/loader.h` → `loader/loaderrunnable.cpp` |
| 快取存取邏輯 | `cache/cache.h`（path → `shared_ptr<Image>`） |
| 縮圖生成 | `thumbnailer/thumbnailerrunnable.cpp` |
| 縮放演算法 | `scaler/scalerrunnable.cpp`（Qt / OpenCV 分支） |
| 目錄排序與過濾 | `directorymanager/directorymanager.cpp` |
| 檔案系統事件 | `directorymanager/watchers/`（Linux: inotify, Windows: ReadDirectoryChanges） |

## KEY CLASSES

| 類別 | 職責 |
|------|------|
| `DirectoryModel` | 唯一對外介面，`Core` 透過它存取所有檔案/影像操作 |
| `DirectoryPresenter` | 驅動縮圖面板與 FolderView，透過 `IDirectoryView` 介面解耦 |
| `Loader` | 優先/非優先兩種非同步載入，回傳 `loadFinished(shared_ptr<Image>)` |
| `Cache` | 執行緒安全，`reserve()`/`release()` 防止快取 item 被提前清除 |
| `Scaler` | 單一執行緒縮放（透過 semaphore 序列化），支援緩衝請求機制 |

## ASYNC PATTERN

所有耗時操作遵循相同模式：
```
XxxRunnable : public QRunnable   // 實際工作
Xxx : public QObject             // 管理 pool，發出 signal 回主執行緒
```
`Runnable` 透過 `QMetaObject::invokeMethod` 或直接 `emit` 回傳結果。

## ANTI-PATTERNS

- 勿繞過 `DirectoryModel` 直接呼叫 `Cache::insert()` 或 `Loader::load()`
- `Scaler` 持有 `Cache*` 指標，勿在 `Scaler` 之外修改同一 cache item 的 pixmap
- 勿在 Runnable 的工作執行緒中存取 Qt GUI 物件
