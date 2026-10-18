# Material Symbols 工具栏图标

Saladin 的工具栏、菜单与对话框图标。图标名的集合是 **Material Symbols Outlined**
（Google Material Design 官方图标语言，Apache-2.0，见 [LICENSE](LICENSE)），与
[qt-material](../themes/qt-material/README.md) 的 Material Design 外观一致。

> qt-material 本身**不提供**工具栏/动作图标，只带样式表与复选框、箭头、分隔条等
> 控件指示器 SVG，所以动作图标由本目录提供。

## 文件

| 文件 | 说明 |
| --- | --- |
| `<name>.svg` | 24px 轮廓图标，单色（`fill="#000000"`），文件名沿用旧的 PNG 图标名，代码无需改动 |
| `material.qrc` | Qt 资源清单，前缀 `/icons/material` |
| `LICENSE` | Material Symbols 的 Apache-2.0 许可证 |

## 运行时着色

[IconLoader](../../utils/iconloader.cpp) 用 `QSvgRenderer` 把 SVG 渲染到目标尺寸，
再用 `CompositionMode_SourceIn` 以当前主题的文字颜色重新着色（
`ThemeHelper::iconColor()`），禁用态用 `ThemeHelper::disabledIconColor()`。因此：

- 一套 SVG 同时适配 `dark_teal` 与 `light_teal`，**不再需要明暗两套 PNG**；
- 图标颜色始终跟随主题，与 Qt-Material 的文字、禁用色一致；
- 尺寸 16 / 22 / 32 / 48 由 `QIcon` 按需要选取，高 DPI 下取上一档。

## 重新生成

需要 Python 3.9+ 与网络（`tmp/` 已 gitignore）：

```powershell
python tools/generate_material_icons.py
```

脚本会：

1. 按 `tools/generate_material_icons.py` 里的 `MAPPING` 从
   [google/material-design-icons](https://github.com/google/material-design-icons)
   下载对应的 `materialsymbolsoutlined/<name>_24px.svg`；
2. 归一化：只保留 `viewBox` 与路径数据，颜色统一为黑色（运行时再着色）；
3. 重写 `material.qrc` 并删除不再使用的旧 SVG。

生成结果需要提交到仓库；只有更换图标或调整映射时才重新运行。

## 未替换的图标

`saladin`（程序图标）等没有 Material 对应物的图标继续使用 `src/icons` 与
`src/icons/dark` 下的 PNG，[IconLoader](../../utils/iconloader.cpp) 会自动回退。