# gui — UI 層

所有使用者介面元件：主視窗、viewer、overlay、面板、對話框、自訂 widget。

## STRUCTURE

```
gui/
├── mainwindow.cpp/h          # 主視窗，持有所有 UI 子元件
├── centralwidget.cpp/h       # 視圖切換中心（document view ↔ folder view）
├── contextmenu.cpp/h/.ui     # 右鍵選單
├── idirectoryview.cpp/h      # 抽象介面（縮圖面板與 FolderView 的共同介面）
├── flowlayout.cpp/h          # 自訂流式佈局
├── viewers/                  # 影像/影片顯示 widget
├── overlays/                 # 浮層（複製/裁切/訊息/影片控制等）
├── panels/                   # 面板（主面板縮圖列、側邊欄、裁切面板、infobar）
├── dialogs/                  # 對話框（設定、重新命名、調整大小、列印等）
├── customwidgets/            # 底層可重用 widget（按鈕、slider、縮圖等）
└── folderview/               # 資料夾瀏覽視圖（實作 IDirectoryView）
```

## WHERE TO LOOK

| 任務 | 位置 |
|------|------|
| 修改主視窗佈局 | `mainwindow.cpp` |
| 修改影像顯示（縮放/平移/FitMode） | `viewers/imageviewerv2.cpp` |
| 影片播放控制 | `viewers/viewerwidget.cpp` + `overlays/videocontrolsproxy.h` |
| 新增浮層訊息 | `overlays/floatingmessageproxy.h`（勿直接操作 `floatingmessage.h`） |
| 設定對話框 | `dialogs/settingsdialog.cpp` |
| 縮圖列/縮圖面板 | `panels/mainpanel/` |
| 資料夾瀏覽視圖 | `folderview/` |
| 裁切 UI | `overlays/cropoverlay.cpp` + `panels/croppanel/` |
| 新增底層 widget 元件 | `customwidgets/` |

## KEY CLASSES

| 類別 | 職責 |
|------|------|
| `MainWindow (MW)` | 持有所有 UI 子元件，處理視窗層級事件（resize, fullscreen, drag-drop） |
| `ImageViewerV2` | 繼承 QGraphicsView，主影像顯示（縮放/平移/動畫/FitMode） |
| `ViewerWidget` | 包裝 `ImageViewerV2` + `VideoPlayer`，切換顯示模式 |
| `DocumentWidget` | 包裝 `ViewerWidget`，提供文件視圖介面 |
| `IDirectoryView` | 抽象介面：`FolderView` 與縮圖面板實作此介面 |

## PROXY PATTERN

overlay 若需延遲初始化，提供 `*Proxy` 類：

```
MainWindow 持有 → FloatingMessageProxy（輕量代理）
                  ↓ 首次使用時建立
                  FloatingMessage（實際 widget）
```

Proxy 清單：`FloatingMessageProxy`, `FullscreenInfoOverlayProxy`, `ImageInfoOverlayProxy`, `VideoControlsProxy`, `ZoomIndicatorOverlayProxy`, `InfoBarProxy`, `FolderViewProxy`

## ANTI-PATTERNS

- 勿直接 `#include` overlay 的 `.h`（繞過 Proxy），應使用對應的 `*proxy.h`
- `ImageViewerV2` 的縮放/平移邏輯不得在 UI 執行緒外觸發（無執行緒安全保證）
- 勿在 `MainWindow` 以外建立新的頂層視窗（應作為 overlay 或 dialog）
