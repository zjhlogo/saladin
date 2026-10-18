# AGENTS.md

本文件是 AI 代理（agent）在本仓库工作时的约定。除特别说明外，所有规则对任何在本项目中执行任务的 agent 都适用。人类开发者可参考，但主要读者是自动化代理。

## 命令执行环境（必须遵守）

**默认且唯一的外壳是 PowerShell 7（pwsh）。** 所有命令一律通过 pwsh 执行，不要使用 bash / sh / cmd。

## 构建与运行

### 环境要求

- Windows 10+，**CMake 3.21+**
- **Qt 6.6+**：Core、Gui、Widgets、Network、Xml、Svg，且需要**私有头文件**（构建链接 `Qt6::CorePrivate` 等）
- 编译器：**MSVC**（Visual Studio 2022+）

### 常用命令（pwsh 语法）

MSVC（在本环境中的首选方式）：

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
cmake --build build --config Release
```

### 注意事项

- 可执行文件输出在**源码树内**（历史 qmake 布局）：Release/RelWithDebInfo/MinSizeRel → `release\saladin.exe`，Debug → `debug\saladin.exe`。
- 每次成功链接后，POST_BUILD 步骤（`cmake/DeployQtRuntime.cmake`）自动把 Qt 运行时 DLL 和插件复制到可执行文件旁，`release/` 与 `debug/` 下的 exe 可直接启动。用 `-DSALADIN_DEPLOY_QT_RUNTIME=OFF` 可禁用。
- VC++ 运行库**不会**被复制；干净机器上需另装 VC++ 可再发行组件包。
- 安装：`cmake --install build --config Release`（安装 `bin/` 与 `translations/` 到前缀目录）。
- `build/`、`debug/`、`release/`、`tmp/`、`.vs/` 均已 gitignore，不要提交。

## 目录结构

```
src/                  应用源码（目标 saladin）
  shell/              Windows Shell 集成：COM/SHLWAPI、PIDL、图标缓存、变更通知
  viewer/             内建查看器：文本/二进制/图像
  utils/              本地设置、数据序列化、编码、更新客户端等
  xmlui/              BSD 许可的 XML UI 库（勿混入 GPL 头，见其 README）
  custom_widgets/     通用自绘控件
  resources/          XML UI 布局（*.xml）与 resources.qrc
  icons/material/     Material Symbols 单色 SVG 图标（工具栏/菜单/对话框），material.qrc
  icons/ + icons/dark 仅保留程序图标 saladin-*.png，各自 .qrc
  guide/              多语言快速指南 HTML + guide.qrc
  themes/qt-material/ Qt-Material 主题：生成的 QSS/指示器 SVG + themes.qrc、手工覆盖样式 saladin.qss
translations/         saladin_*.ts / *.qm、locale.ini
cmake/                DeployQtRuntime.cmake
tools/                开发期辅助脚本（如 Qt-Material 资源生成），不参与构建
```

## 代码约定

- **新源文件**必须登记到 `src/CMakeLists.txt`：源码加进 `SALADIN_SOURCES`，头文件加进 `SALADIN_HEADERS`，放在对应分组注释（`--- shell/ ---` 等）之下；新 `.qrc` 加进 `SALADIN_RESOURCES`。
- **moc/uic/rcc**：工程开启 AUTOMOC/AUTOUIC/AUTORCC。含 `Q_OBJECT` 的头必须已在 `SALADIN_HEADERS` 中列出才会被 moc 处理。
- **UI 构建**：菜单/工具条/动作由 XML（`src/resources/*.xml`）+ `xmlui` 库生成，不是 Qt Designer `.ui` 文件；改 UI 通常是"XML + 对应类"两处配合。
- **图标**：工具栏/菜单/对话框图标是 `src/icons/material/` 下的 Material Symbols 单色 SVG（与 Qt-Material 的 Material Design 风格一致），运行时由 `IconLoader` 按当前主题文字色着色，**一套即可覆盖明暗主题**。文件名就是 `IconLoader::icon("...")` 用的图标名；新增或替换图标时改 `tools/generate_material_icons.py` 里的 `MAPPING` 并重新生成，详见 `src/icons/material/README.md`。`src/icons`、`src/icons/dark` 只保留程序图标 `saladin-*.png`。
- **翻译**：用户可见字符串一律 `tr()`。新增语言：新建 `translations/saladin_<lang>.ts`，注册到 `SALADIN_LANGUAGES`（`CMakeLists.txt`）与 `translations/locale.ini`。更新译文后构建 `saladin_lrelease` 目标（或整目标 `release_translations`）重新生成 `.qm`。
- **文件命名**：`snake_case.cpp/.h`（`custom_widgets/PathLineEditor.*` 为历史例外）；类名 PascalCase。
- **shell/ 子系统**：大量 COM 对象与 PIDL 手工生命周期管理，修改时注意 AddRef/Release 配对与 `::CoTaskMemFree` 的使用。
