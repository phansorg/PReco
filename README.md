# preco

## 使用ライブラリ
- JSON for Modern C++
  - https://github.com/nlohmann/json
  - `preco/include/nlohmann/json.hpp`の階層になるよう、ヘッダファイルを配置
- OpenCV
  - https://github.com/opencv/opencv
  - `preco/include/opencv2/opencv.hpp`の階層になるよう、include配下のヘッダファイルを全て配置
  - `preco/lib/opencv/opencv_world460.lib`の階層になるよう、`opencv_world460.lib`と`opencv_world460d.lib`を配置(d有りはDebugビルド用)
- spdlog
  - https://github.com/gabime/spdlog
  - `preco/include/spdlog/spdlog.h`の階層になるよう、include配下のヘッダファイルを全て配置

## ビルド後のdll配置
- OpenCV
  - Releaseビルド
    - `preco/x64/Release/opencv_world460.dll`の階層になるよう、dllファイルを配置
  - Debugビルド
    - `preco/x64/Debug/opencv_world460d.dll`の階層になるよう、dllファイルを配置(d有りはDebugビルド用)
