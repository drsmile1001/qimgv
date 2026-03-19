# gui/customwidgets — 底層可重用 Widget

所有自訂 Qt widget 元件，供 overlays、panels、dialogs、mainwindow 直接使用。

## FILES

| 檔案 | 職責 |
|------|------|
| `thumbnailview.h/cpp` | 縮圖滾動列表（QAbstractScrollArea），縮圖面板與 FolderView 的基底 |
| `thumbnailwidget.h/cpp` | 單一縮圖 widget（顯示圖示/名稱/星評） |
| `thumbnailwidgetcmp.h/cpp` | 縮圖比較 widget（雙圖並排） |
| `slidepanel.h/cpp` | 滑動展開面板容器（主面板、側邊欄） |
| `floatingwidget.h/cpp` | 浮動 widget 基類（overlay 的底層基類） |
| `floatingwidgetcontainer.h/cpp` | 管理多個 FloatingWidget 的容器 |
| `overlaywidget.h/cpp` | Overlay 基類（FloatingWidget 子類，帶半透明背景） |
| `sidepanelwidget.h/cpp` | 側邊欄 widget 基類 |
| `videoslider.h/cpp` | 影片進度條（支援拖曳 seek） |
| `actionbutton.h/cpp` | 可綁定 ActionID 的按鈕 |
| `iconbutton.h/cpp` | 圖示按鈕（svg recolor 支援） |
| `keysequenceedit.h/cpp` | 快捷鍵輸入框（設定對話框用） |
| `styledcombobox.h/cpp` | 套用主題的 combobox |
| `colorselectorbutton.h/cpp` | 顏色選擇按鈕 |
| `menuitem.h/cpp` | 選單項目 widget |
| `ssidebar.h/cpp` | 設定側邊欄 widget |

## CONVENTIONS

- 繼承鏈：`QWidget` → `FloatingWidget` → `OverlayWidget` → 各 overlay 實作
- Icon recolor 透過 `ImageLib::recolor(QPixmap&, QColor)` 完成，勿硬編碼圖示顏色
- `ActionButton` 透過 `ActionManager` 綁定 ActionID，勿直接連接 clicked 到業務邏輯

## ANTI-PATTERNS

- 勿在此目錄的 widget 中直接 `#include "core.h"` 或持有 `Core*`
- 勿在 widget 建構子中做耗時操作（影像載入、IO）
