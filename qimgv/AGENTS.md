# qimgv 主應用模組

主應用源碼根目錄。所有業務邏輯、UI、後端元件皆在此。

## STRUCTURE

```
qimgv/
├── core.cpp/h            # 頂層協調者（唯一連接所有子系統的入口）
├── main.cpp              # 應用入口，建立 QApplication + Core
├── settings.cpp/h        # 全域單例設定 + 所有枚舉定義
├── themestore.cpp/h      # 主題色彩存儲
├── sharedresources.cpp/h # 共享靜態資源（圖示快取等）
├── shortcutbuilder.cpp/h # 快捷鍵字串解析
├── proxystyle.cpp/h      # Qt style 代理（自訂繪製）
├── starrating.cpp/h      # 星評功能
├── components/           # 後端子系統 → 見 components/AGENTS.md
├── gui/                  # UI 層 → 見 gui/AGENTS.md
├── sourcecontainers/     # 影像資料模型 → 見 sourcecontainers/AGENTS.md
├── utils/                # 工具類 → 見 utils/AGENTS.md
├── 3rdparty/QtOpenCV/    # OpenCV/QImage 轉換橋接（僅 USE_OPENCV 時編譯）
├── res/                  # 圖示、翻譯 .ts/.qm、.qrc
└── tests/                # 單元測試（目前僅 MapOverlay 測試）
```

## KEY FILES

| 檔案 | 職責 |
|------|------|
| `core.cpp` | `connectComponents()` 負責所有主要 signal/slot 連接，是理解資料流的入口 |
| `settings.h` | 所有枚舉定義集中於此（`SortingMode`, `ImageFitMode`, `ScalingFilter` 等） |
| `main.cpp` | 解析命令列參數，建立 `Core`，呼叫 `core.showGui()` |

## CONVENTIONS

- `Core` 是唯一橋接 model 與 GUI 的類別，不得在其他地方直接跨層連接
- 新增枚舉必須放在 `settings.h`，不得分散到各子目錄
- `StarRating` 是 `Core` 的直接成員（`Core::starRating`），特例

## ANTI-PATTERNS

- 勿在 `core.h` 以外的地方同時 `#include` model 和 GUI 的頭文件並建立連接
- 勿修改 `3rdparty/` 代碼（上游維護）
- `tests/` 目前需要 QtTest，LSP 可能報錯（已知問題，非 `compile_commands.json` 範疇）
