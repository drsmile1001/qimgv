# plugins/player_mpv — MPV 影片播放外掛

獨立的動態庫（MODULE），由主程式透過 `QPluginLoader`/dlopen 載入。

## STRUCTURE

```
player_mpv/
├── CMakeLists.txt        # 獨立 CMake 目標（MODULE library），連結 libmpv
└── src/
    ├── videoplayermpv.h/cpp      # 主要實作（使用 OpenGL 整合 mpv）
    └── nogl/
        └── videoplayermpv.h/cpp  # 不使用 OpenGL 的備用實作
```

## HOW IT'S LOADED

主程式以宏定義路徑載入：

```cpp
// 由 CMake 定義
_QIMGV_PLUGIN_DIR  // 外掛目錄（Linux: 系統 lib 路徑, Windows: "plugins"）
_QIMGV_PLAYER_PLUGIN  // "player_mpv.so" 或 "player_mpv.dll"
```

守衛宏：`#ifdef USE_MPV`（由 `-DVIDEO_SUPPORT=ON` 啟用）

## BUILD

```bash
# 含影片支援
cmake -S . -B build -G Ninja -DVIDEO_SUPPORT=ON
ninja -C build

# 外掛安裝路徑
sudo ninja -C build install  # 安裝到系統 lib 目錄
```

## CONVENTIONS

- 外掛實作 `VideoPlayerInterface`（在 `qimgv/gui/viewers/videoplayer.h`）
- `nogl/` 版本用於沒有 OpenGL 環境的系統（如部分伺服器）
- 勿在此外掛中 `#include` 主程式的頭文件（除 interface 外）

## ANTI-PATTERNS

- 勿直接在主程式中 `#include "videoplayermpv.h"`（破壞外掛隔離）
- 勿在 VIDEO_SUPPORT=OFF 環境中假設外掛已載入
