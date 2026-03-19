# gui/folderview — 資料夾瀏覽視圖

`IDirectoryView` 介面的主要實作，提供縮圖格狀瀏覽、書籤、檔案系統樹，以及拖放支援。

## FILES

| 檔案 | 職責 |
|------|------|
| `folderview.h/cpp` + `.ui` | 主視圖容器（實作 `IDirectoryView`），整合所有子元件 |
| `folderview.ui` | Qt Designer 佈局（含 splitter、縮圖格、工具列） |
| `foldergridview.h/cpp` | 縮圖格狀顯示（繼承 `ThumbnailView`），處理選取與拖放 |
| `filesystemmodelcustom.h/cpp` | 自訂 `QFileSystemModel`，過濾非目錄項目 |
| `bookmarkswidget.h/cpp` | 書籤面板（書籤清單 + 新增按鈕） |
| `bookmarksitem.h/cpp` | 單一書籤項目 widget |
| `treeviewcustom.h/cpp` | 自訂 `QTreeView`（檔案系統樹，用於路徑導覽） |
| `fvoptionspopup.h/cpp` + `.ui` | 排序/顯示選項彈出視窗 |

## INTERFACE IMPLEMENTATION

`FolderView` 實作 `IDirectoryView`（`gui/idirectoryview.h`）的全部方法：

| 方法 | 行為 |
|------|------|
| `populate(int count)` | 重建縮圖格，設定項目總數 |
| `setThumbnail(pos, thumb)` | 更新指定位置的縮圖 widget |
| `select(int)` / `select(QList<int>)` | 設定選取項目（高亮顯示） |
| `focusOn(int)` | 捲動至指定項目 |
| `setDirectoryPath(path)` | 更新路徑列 + 檔案系統樹 |
| `insertItem(index)` / `removeItem(index)` | 動態新增/移除縮圖項目 |

## LAYOUT

```
FolderView（QWidget, FloatingWidgetContainer）
├── QSplitter
│   ├── 左側：BookmarksWidget + TreeViewCustom（可切換顯示）
│   └── 右側：FolderGridView（ThumbnailView 子類）
└── 工具列：路徑 combobox、排序、選項彈出、首頁/根目錄按鈕
```

## CONVENTIONS

- `FolderView` 由 `FolderViewProxy`（`gui/overlays/folderviewproxy.h`）懶載入
- `DirectoryPresenter` 持有 `IDirectoryView*` 指標，不直接依賴 `FolderView`
- 縮圖請求透過 `thumbnailsRequested(QList<int>, int, bool, bool)` signal 發出
- 拖放到書籤觸發 `draggedToBookmarks(QList<int>)` → `Core` → `FileOperations`
- `.ui` 佈局不得手動 `addWidget` 覆蓋（使用 `ui->` accessor）

## ANTI-PATTERNS

- 勿在 `FolderView` 中直接存取 `DirectoryModel`（透過 `DirectoryPresenter` signal/slot）
- 勿跳過 `FolderViewProxy` 直接實例化 `FolderView`
- `FolderGridView` 繼承 `ThumbnailView`，勿繞過基類直接操作 item list
