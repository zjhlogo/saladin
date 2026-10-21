# Saladin

**Saladin** 是一款免费、开源的 Windows 双栏文件管理器，具有经典 Norton Commander 风格应用程序的神韵。它以紧凑的工具条取代传统菜单栏，提供两个可独立浏览的面板、标签页、书签和内建查看器。

Saladin 由 Michał Męciński 创建（2011-2017）。本仓库包含对原始代码库的现代化改造，已从 Qt 5/qmake 移植到 **Qt 6 + CMake**。

## 功能特性

- **双栏与标签页** —— 一个活动（源）面板和一个目标面板，每个面板都有驱动器条、位置栏和文件列表；可切换、互换、克隆和同步面板。
- **Windows Shell 深度集成** —— 文件操作使用与 Windows 资源管理器相同的引擎（带进度与冲突处理的复制/移动、删除到回收站、拖放、资源管理器上下文菜单），并具备实时图标和目录变更通知。
- **内建查看器** —— 支持编码检测的文本查看、带跳转功能的二进制/十六进制视图，以及图像查看器（缩放、旋转、全屏）。
- **工具** —— 文件搜索、支持掩码和正则表达式的批量重命名、ZIP 打包、FTP 浏览、大小统计、文件与目录比较，以及可配置的外部工具（编辑器、控制台、diff、查看器）。
- **书签与历史记录**，方便访问常用位置。
- **插件框架** —— 支持用 C++ + Qt 编写原生插件（DLL）扩展工具条、菜单与文件右键菜单，并可读写双栏面板；见[插件开发指南](plugins/README.md)。
- **明暗主题**，基于 [qt-material](https://github.com/dunderlab/qt-material)（BSD-2-Clause）样式表，含深色图标集；可在设置中跟随系统或强制明暗。
- **多语言翻译**：德语、英语、西班牙语、匈牙利语、波兰语、巴西葡萄牙语、俄语。

内建的**快速指南**（帮助 → 快速指南）介绍了所有功能、键盘快捷键、通配符、重命名掩码和正则表达式。

## 从源码构建

### 环境要求

- **Windows** 10 或更高版本
- **Qt 6.6+**，包含 *Core、Gui、Widgets、Network、Xml、Svg* 模块 —— 包括开发头文件（构建需要 Qt 私有头文件）
- **CMake 3.21+**
- 编译器：**MSVC**（Visual Studio 2022 或更高版本）或 **MinGW-w64**

### 配置与构建

MSVC（Visual Studio 生成器）：

```bat
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
cmake --build build --config Release
```

MinGW（在普通命令提示符下使用 Ninja 最快）：

```bat
set PATH=C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64
cmake --build build --parallel
```

### 输出与运行

沿用历史 qmake 布局，可执行文件会写入**源码树内**的 `release/`（Release、RelWithDebInfo、MinSizeRel）或 `debug/`（Debug）目录。每次成功链接后，构建过程会把 Qt 运行时 DLL、Saladin 所用的插件以及 `translations/` 下的运行时语言文件（`saladin_*.qm`、`locale.ini`）复制到可执行文件旁（语言文件位于 `<exe 目录>\translations`），因此 `release\saladin.exe` 和 `debug\saladin.exe` 都可以在没有 Qt 环境的情况下直接启动，并直接使用目标路径下的语言文件。注意：

- 使用 `-DSALADIN_DEPLOY_QT_RUNTIME=OFF` 可禁用此行为。
- Visual C++ 运行库**不会**被复制；要在干净的机器上运行 MSVC 构建，请安装 Visual C++ 可再发行组件包。

### 安装

```bat
cmake --install build --config Release
```

这会把 `bin/` 和 `translations/` 安装到配置的前缀目录。

## 主题

界面外观由 [qt-material](https://github.com/dunderlab/qt-material) 的 `dark_teal` / `light_teal` 样式表提供（见 `src/themes/qt-material/`）：`Dark` 始终使用深色主题，`Light` 始终使用浅色主题，`Follow System` 跟随 Windows 的明暗设置并在运行时自动切换。

- 主题颜色定义在 `src/themes/qt-material/themes/*.xml`，[src/ThemeHelper.cpp](src/ThemeHelper.cpp) 在运行时据此构建 `QPalette`，并加载对应的 `.qss` 与紧凑化覆盖样式 `saladin.qss`。
- 生成产物（QSS、指示器 SVG、`themes.qrc`）已提交到仓库，**正常构建不需要 Python**。
- 需要更换主色或升级 qt-material 时，按 [src/themes/qt-material/README.md](src/themes/qt-material/README.md) 重新生成资源。

## 翻译

可翻译字符串位于 `translations/saladin_*.ts`。安装了 Qt *Linguist* 工具时会自动生成编译后的 `.qm` 文件（目标 `saladin_lrelease`；否则使用仓库中已提交的 `.qm` 文件）。运行时语言文件（`saladin_*.qm`、`locale.ini`）会随构建被复制到可执行文件旁的 `translations/` 子目录，程序启动时优先使用该副本；只有在它不存在时才回退到可执行文件上一级的 `translations/`（`cmake --install` 的 `bin/` + `translations/` 布局）。要新增语言，请新建 `.ts` 文件，并在 `SALADIN_LANGUAGES`（`CMakeLists.txt`）和 `translations/locale.ini` 中注册。

代码变更后重新收集源码中的字符串：`cmake --build build --target saladin_lupdate`（lupdate 会基于 `saladin` 目标的源码列表刷新全部 `.ts`，并自动清理已删除字符串对应的废弃条目；等价的全局目标为 `update_translations`）。仅重新编译语言文件（`.ts` → `.qm`）：`cmake --build build --target saladin_lrelease`。

## 许可证

Saladin 是自由软件：您可以根据自由软件基金会发布的 **GNU 通用公共许可证**（第 **3** 版，或您自选的任何更高版本）重新分发和/或修改它。完整许可文本见
<https://www.gnu.org/licenses/gpl-3.0.html>。

- 版权所有 © 2011-2017 Michał Męciński
- 图标版权所有 © 2017 Łukasz Grabowski
- `src/xmlui/` 是内置的 BSD 许可 XML UI 库 —— 参见 `src/xmlui/README`
- `src/themes/qt-material/` 包含 [qt-material](https://github.com/dunderlab/qt-material) 的 BSD-2-Clause 样式表与图标资源 —— 参见 `src/themes/qt-material/LICENSE`
