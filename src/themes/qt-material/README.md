# Qt-Material 主题资源

本目录包含 Saladin 使用的 [qt-material](https://github.com/dunderlab/qt-material)（PyPI 包名 `qt-material`，BSD-2-Clause，见 [LICENSE](LICENSE)）生成产物。运行时由 [src/ThemeHelper.cpp](../../ThemeHelper.cpp) 直接加载，**构建过程不需要 Python**。

## 内容

| 文件 | 说明 |
| --- | --- |
| `dark_teal.qss` | 深色主题样式表（`Dark` 使用） |
| `light_teal.qss` | 浅色主题样式表（`Light` 使用，`invert_secondary=True`） |
| `saladin.qss` | 手工维护的紧凑化覆盖样式，追加在主题 QSS 之后；`@TOKEN@` 由 `ThemeHelper` 用主题颜色替换 |
| `themes/*.xml` | qt-material 主题色定义；`primaryTextColor` 已固定为在强调色上可读的深色 |
| `<theme>/{primary,active,disabled}/*.svg` | 复选框、单选框、下拉箭头、树展开箭头等指示器图标 |
| `themes.qrc` | Qt 资源清单，把上述文件编入可执行文件 |

qt-material **不提供**工具栏/菜单/动作图标（上游只带样式表、字体与上面这些控件指示器 SVG）；Saladin 的动作图标使用同一设计语言的 [Material Symbols](../icons/material/README.md)。

主题与 Saladin 选项的对应关系：

- `Dark` → `dark_teal`
- `Light` → `light_teal`
- `Follow System` → 按 Windows 明暗在两者之间选择

## 重新生成

需要 Python 3.9+；在仓库根目录执行（`tmp/` 已被 gitignore）：

```powershell
python -m pip install --target tmp/qtmaterial "qt-material==2.17" "PySide6-Essentials==6.12.0"
$env:PYTHONPATH = "$PWD\tmp\qtmaterial"
python tools/generate_qt_material_theme.py
```

脚本会：

1. 从 PyPI 包的 `qt_material/themes/` 复制主题 XML，并把 `primaryTextColor` 固定为可读的深色；
2. 用 `export_theme()` 生成 QSS 和 SVG，图标 URL 前缀为 `:/themes/qt-material/<theme>/`；
3. 清理 Qt 不支持的全局字体、`text-transform` 和 `unset` 声明（避免覆盖查看器字体、减少样式表告警）；
4. 重写 `themes.qrc`。

生成结果需要提交到仓库；只有更换主题或升级 qt-material 时才重新运行。

## 自定义

- 更换主色：把 `tools/generate_qt_material_theme.py` 中的主题对改为例如 `dark_blue` / `light_blue`，同时更新 `src/ThemeHelper.cpp` 中的主题名，然后重新生成。
- 调整文件管理器紧凑布局：直接编辑 `saladin.qss`，无需重新生成 qt-material 产物。