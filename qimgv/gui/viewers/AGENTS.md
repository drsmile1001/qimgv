# gui/viewers — 影像與影片顯示層

影像/動畫/影片三種內容的顯示核心，包含縮放、平移、播放控制與外掛載入。

## FILES

| 檔案 | 職責 |
|------|------|
| `imageviewerv2.h/cpp` | 主影像顯示（QGraphicsView），縮放/平移/動畫/FitMode |
| `viewerwidget.h/cpp` | 包裝 `ImageViewerV2` + `VideoPlayerInitProxy`，依內容切換顯示模式 |
| `documentwidget.h/cpp` | 包裝 `ViewerWidget`，提供文件視圖介面給 `MainWindow` |
| `videoplayer.h/cpp` | 影片播放器抽象介面（純虛類，由外掛實作） |
| `videoplayerinitproxy.h/cpp` | 影片外掛懶載入 proxy，透過 `QLibrary` 動態載入 `player_mpv.so` |

## CLASS HIERARCHY

```
QGraphicsView → ImageViewerV2
FloatingWidgetContainer → ViewerWidget（持有 ImageViewerV2 + VideoPlayerInitProxy）
FloatingWidgetContainer → DocumentWidget（持有 ViewerWidget）
QObject → VideoPlayer（純虛）← VideoPlayerMpv（外掛實作）
VideoPlayer → VideoPlayerInitProxy（proxy，主程式持有）
```

## KEY BEHAVIORS

`ImageViewerV2`：
- `showImage(unique_ptr<QPixmap>)` → 先呼叫 `reset()`（會短暫清空場景，可能黑閃），再設新 pixmap
- `scalingRequested(QSize, ScalingFilter)` signal → 由 `Core` 轉發給 `Scaler`
- `setScaledPixmap()` → 接收 `Scaler` 回傳的高品質 pixmap，替換低品質快速縮放
- 兩個 pixmap 場景物件並存：`pixmapItem`（原始）、`pixmapItemScaled`（高品質縮放結果）
- `ViewLockMode`：`LOCK_NONE` / `LOCK_ZOOM` / `LOCK_ALL`，透過 `toggleLockZoom()`/`toggleLockView()` 切換

`ViewerWidget`：
- `enableImageViewer()` / `enableVideoPlayer()` 管理兩個子 widget 的顯示/隱藏
- `showVideo()` 觸發外掛懶初始化（首次呼叫 `VideoPlayerInitProxy::initPlayer()`）
- 持有 `VideoControlsProxyWrapper` + `ZoomIndicatorOverlayProxy` + `ClickZoneOverlay`

`VideoPlayerInitProxy`：
- 首次需要時載入 `_QIMGV_PLUGIN_DIR/_QIMGV_PLAYER_PLUGIN`（由 CMake 定義）
- 若外掛載入失敗，靜默降級（不崩潰）
- 守衛宏：`#ifdef USE_MPV`

## ANTI-PATTERNS

- `ImageViewerV2::showImage()` 呼叫 `reset()` 前勿假設場景狀態完整（會清除所有場景項目）
- 勿直接 `#include "videoplayermpv.h"`，只能透過 `VideoPlayerInitProxy`
- 勿在 `ImageViewerV2` 外部觸發縮放計算（非執行緒安全）
- 縮放 signal 鏈：`ImageViewerV2::scalingRequested` → `ViewerWidget::scalingRequested` → `Core` → `Scaler`，勿短路
