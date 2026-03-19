# utils — 工具類集合

靜態工具方法與跨層共用輔助類。不含業務邏輯，不依賴 UI 執行緒假設。

## FILES

| 檔案 | 職責 |
|------|------|
| `imagelib.h/cpp` | 靜態影像操作（旋轉、裁切、翻轉、縮放） |
| `imagefactory.h/cpp` | 從路徑建立 `Image` 子類實例的工廠 |
| `fileoperations.h/cpp` | 複製/移動/刪除/重新命名，回傳 `FileOpResult` 枚舉 |
| `inputmap.h/cpp` | 鍵盤/滑鼠輸入 → ActionID 的映射表 |
| `actions.h/cpp` | ActionID 枚舉與字串化 |
| `script.h/cpp` | 腳本物件（名稱 + 命令/路徑）|
| `cmdoptionsrunner.h/cpp` | 命令列選項處理（--gen-thumbs 等） |
| `wallpapersetter.h/cpp` | 跨平台桌布設定 |
| `randomizer.h/cpp` | 隨機索引生成（shuffle 模式） |
| `numeric.h` | 數值工具（clamp 等） |
| `stuff.h/cpp` | 雜項（`DocumentType` 偵測、MIME 等） |

## KEY PATTERNS

`ImageLib` — 純靜態方法類，所有操作返回 `QImage*`（呼叫方負責釋放）：

```cpp
// 正確使用
QImage* rotated = ImageLib::rotated(src, 90);  // src 是 shared_ptr<const QImage>
// rotated 使用後 delete
```

`FileOpResult` — 所有檔案操作的回傳枚舉，勿忽略：

```cpp
FileOpResult result;
model->copyFileTo(src, dest, false, result);
if (result != FileOpResult::SUCCESS) { /* handle */ }
```

## CONVENTIONS

- `ImageLib` 所有方法為 `static`，不需實例化
- `Raw` 後綴方法（如 `rotatedRaw`）接受裸 `QImage*`，用於效能敏感路徑
- OpenCV 路徑在 `#ifdef USE_OPENCV` 包裹

## ANTI-PATTERNS

- `ImageLib` 返回的 `QImage*` 必須被 caller 管理，勿洩漏
- 勿在此目錄新增含 Qt signal/slot 的類（應放 `components/`）
- 勿直接呼叫 `FileOperations` 靜態方法繞過 `DirectoryModel`（會跳過快取同步）
