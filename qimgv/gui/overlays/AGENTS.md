# gui/overlays — 浮層元件

所有顯示在影像或視窗上層的浮動 UI 元件，以及其 Proxy 包裝類。

## FILES

| 浮層 | Proxy | 職責 |
|------|-------|------|
| `floatingmessage` | `floatingmessageproxy` | 暫態通知訊息（操作回饋） |
| `fullscreeninfooverlay` | `fullscreeninfooverlayproxy` | 全螢幕時顯示檔案資訊 |
| `imageinfooverlay` | `imageinfooverlayproxy` | EXIF 資訊面板（按 I 鍵顯示） |
| `videocontrols` | `videocontrolsproxy` | 影片播放控制列 |
| `zoomindicatoroverlay` | `zoomindicatoroverlayproxy` | 縮放比例指示器 |
| `cropoverlay` | — | 裁切選框（拖拉選取區域） |
| `copyoverlay` | — | 快速複製/移動目錄面板 |
| `saveconfirmoverlay` | — | 儲存確認提示 |
| `renameoverlay` | — | 重新命名輸入框 |
| `controlsoverlay` | — | 滑鼠懸停時顯示的控制按鈕 |
| `clickzoneoverlay` | — | 點擊區域（上一張/下一張熱點） |
| `mapoverlay` | — | 地圖位置顯示（GPS EXIF） |
| `changelogwindow` | — | 更新日誌視窗 |

## PROXY PATTERN 使用規則

- `MainWindow` 持有所有 Proxy（非實際 widget）
- 只有在第一次使用時 Proxy 才建立實際 widget
- 從 `Core` 或外部呼叫時，永遠透過 Proxy 的 slot/signal

```cpp
// 正確：透過 proxy
mw->floatingMessage->showMessage("已儲存", FloatingMessage::Ok);

// 錯誤：繞過 proxy 直接建立
auto *msg = new FloatingMessage(parent);
```

## CONVENTIONS

- `.ui` 文件存在代表使用 Qt Designer 佈局（不得手動 addWidget 覆蓋 .ui 的佈局）
- 所有 overlay 繼承自 `OverlayWidget`（在 `customwidgets/`）

## ANTI-PATTERNS

- 勿在 overlay 中持有 `DirectoryModel` 或 `Cache` 的指標
- 勿讓 overlay 發出直接修改檔案的 signal（應透過 `MainWindow` → `Core`）
