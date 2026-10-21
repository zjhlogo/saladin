# Saladin 插件开发指南

Saladin 支持用 **C++ + Qt** 编写原生插件（DLL）来扩展程序功能。插件由宿主在启动时通过 `QPluginLoader` 加载，跨边界通信只使用 [`src/plugin/plugin_api.h`](../src/plugin/plugin_api.h) 中的纯虚接口，**插件不链接 saladin.exe**，只依赖 Qt 与这一个头文件。

> **安全提示**：原生插件与 Saladin 运行在同一进程、同一用户权限下。只安装你信任来源的插件。

## 插件能做什么（当前 API 版本 1.0）

- **贡献动作到主窗口**：把自己的 `QAction` 合并进主工具条与菜单（复用 `XmlUi` 合并机制），例如工具条 `sectionTools` 区、菜单栏 `menuBarTools`、各弹出菜单等；
- **注入文件右键菜单**：在文件列表的原生 Shell 右键菜单末尾追加 Qt 动作（可按选中内容动态调整文本与可用性）；
- **读写双栏面板**：查询当前目录、条目、选中项，跳转目录，刷新面板；
- **插件私有设置**：键自动加 `Plugin.<插件id>.` 前缀，持久化在用户 `settings.dat`，跨会话保留；
- **可选设置页**：在设置对话框的“插件”页嵌入自己的配置控件；
- **主题图标**：经 `PluginHost::themeIcon()` 取与当前主题着色一致的内置 Material 图标。

暂不支持：面板事件回调（目录切换/选区变化通知）、自定义查看器类型、跨线程调用。所有 API 必须在 **GUI 线程**调用。

## 插件目录

宿主按顺序扫描两个目录，**同 id 先到先得**（系统目录优先）：

1. 系统目录：`<exe 目录>\plugins`（安装器分发）
2. 用户目录：`<AppData>\Saladin\plugins`（设置对话框 → 插件 → “打开插件目录”）

`.dll` 文件旁需要（或内嵌）`metadata.json`。在设置对话框中停用的插件记录在 `DisabledPlugins`，**重启后生效**；运行期宿主不会卸载插件 DLL。

## 最小示例

一个插件 = 一个继承 `QObject` + `Saladin::Plugin` 的类 + 元数据 JSON + 可选的 UI XML。完整可编译的参考实现在 [`plugins/example/`](example/)。

### 1. 插件类

```cpp
#include "plugin/plugin_api.h"

class MyPlugin : public QObject, public Saladin::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SALADIN_PLUGIN_IID FILE "metadata.json")
    Q_INTERFACES(Saladin::Plugin)

public:
    bool initialize(Saladin::PluginHost* host) override
    {
        m_host = host;           // 保存即可，指针在 shutdown() 前一直有效
        return true;             // 返回 false 表示初始化失败
    }

    void shutdown() override { m_host = nullptr; }

    QList<QAction*> createActions(QObject* parent) override
    {
        auto action = new QAction(tr("My Action"), parent);
        action->setObjectName("myplugin.run");   // 必须 "<插件id>.<名称>"
        connect(action, &QAction::triggered, this, &MyPlugin::run);
        return { action };
    }

    QList<QAction*> contextMenuActions(const QList<Saladin::FileInfo>& items) override
    {
        if (items.isEmpty()) return {};          // 空白处右键或未选中
        auto action = new QAction(tr("My Action"), this);
        connect(action, &QAction::triggered, this, &MyPlugin::run);
        return { action };                       // 所有权归插件，宿主不删除
    }
};
```

### 2. metadata.json（与 DLL 同目录，`Q_PLUGIN_METADATA(FILE ...)` 指向它）

```json
{
    "name": "myplugin",
    "displayName": "My Plugin",
    "version": "1.0.0",
    "apiVersion": "1.0",
    "author": "you",
    "description": "一句话描述。",
    "ui": "plugin.xml"
}
```

- `name` 是插件唯一 id，前缀约定与设置页显示都依赖它；
- `apiVersion` 必须等于 `SALADIN_PLUGIN_API_VERSION`（当前 `"1.0"`），否则宿主拒绝加载；
- `displayName` 可省略（缺省显示 `name`）。

### 3. plugin.xml（可选，动作的摆放位置）

格式与 [`src/resources/mainwindow.xml`](../src/resources/mainwindow.xml) 相同。引用宿主已存在的容器 id，`XmlUi::Builder` 会把元素**合并**进对应位置：

| 容器 id | 位置 |
| --- | --- |
| `stripMain` | 主工具条；引用其中的 `sectionClipboard/sectionRefresh/sectionHidden/sectionPanels/sectionSelect/sectionFunctions/sectionTools` 可并入既有分区 |
| `menuBarEdit/Navigate/View/Select/Functions/Tools/Help` | 菜单栏各菜单 |
| `menuView/menuEdit/menuCopy/menuMove/menuDelete/menuEditCopy/menuOpen/menuCompare` | 工具条按钮的弹出菜单 |
| `menuDrives` | 驱动器条右键菜单 |

```xml
<xmlui>
  <strip id="stripMain">
    <section id="sectionTools">
      <action id="myplugin.run"/>
    </section>
  </strip>
  <menu id="menuBarTools">
    <action id="myplugin.run"/>
  </menu>
</xmlui>
```

- `<action id>` 必须与 `createActions()` 返回动作的 `objectName` 完全一致；
- 也可以声明新的 `<menu id="myplugin.xxx">` 子菜单，但那不会自动显示，除非把它挂到你已有的某个条目下；
- `ui` 字段缺省时动作不会出现在主窗口（右键菜单注入不受影响）。

### 4. CMake

```cmake
qt_add_plugin(myplugin SHARED myplugin.h myplugin.cpp metadata.json plugin.xml)
target_include_directories(myplugin PRIVATE "<Saladin 源码目录>/src")
target_link_libraries(myplugin PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(myplugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "<Saladin 源码目录>/release/plugins")
```

插件与宿主必须使用**同一套 Qt 与同一编译器配置**（MSVC Release 对 Release，Debug 对 Debug）。Debug/Release 或位数不匹配时插件会加载失败，错误会显示在设置对话框的插件页中，不影响宿主运行。

## API 参考

`Saladin::Plugin`（宿主回调）：

| 方法 | 说明 |
| --- | --- |
| `initialize(PluginHost*)` | 加载后调用一次；返回 `false` 记为失败 |
| `shutdown()` | 程序退出前调用一次；之后不要再使用任何宿主指针 |
| `createActions(QObject*)` | 贡献主窗口动作；`objectName` 必须 `<插件id>.` 前缀；动作所有权归宿主 |
| `contextMenuActions(items)` | 右键菜单追加项；按选中内容调用；所有权归插件 |
| `createSettingsWidget(parent)` | 可选；返回的控件由宿主销毁，不要缓存指针 |

`Saladin::PluginHost`（宿主服务）：

| 方法 | 说明 |
| --- | --- |
| `saladinVersion()` | 程序版本号 |
| `mainWindow()` | 主窗口，作对话框父窗口 |
| `sourcePanel()` / `targetPanel()` | 活动/目标面板，随面板切换动态变化 |
| `themeIcon(name)` | 主题着色的内置图标（名称见 `src/icons/material/`） |
| `settingsValue()` / `setSettingsValue()` | 插件私有设置，键自动加前缀 |

`Saladin::PanelApi`（单侧面板）：

| 方法 | 说明 |
| --- | --- |
| `isActive()` | 是否为当前活动面板 |
| `folderPath()` | 当前目录路径（虚拟位置可能为空） |
| `items()` / `selectedItems()` / `currentItem()` | 条目快照，见 `FileInfo` |
| `setDirectory(path)` | 跳转目录，返回是否生效 |
| `refresh()` | 强制刷新 |

`Saladin::FileInfo`：`name`、`path`、`size`、`lastModified`、`directory`、`hidden`、`readOnly`。

## 行为细节与排错

- **加载失败**：缺依赖 DLL、架构不符、Qt Debug/Release 混用、非 Qt 插件等都会在设置 → 插件页显示具体 `errorString()`；程序不受影响。
- **API 不匹配**：显示“不兼容”，需要更新插件或 Saladin。
- **动作没出现在主窗口**：检查 `objectName` 前缀与 plugin.xml 的 `action id` 是否一致、`metadata.json` 是否配置了 `ui`。
- **右键菜单项为灰色**：动作处于禁用态，宿主按 `isEnabled()` 渲染。
- **异常**：`initialize`/`shutdown`/`contextMenuActions` 中的异常会被宿主捕获并跳过该插件，但不要依赖这一点。
- **翻译**：插件用自己的 `.qm`（`QTranslator`），宿主不管理插件翻译。
- **图标随主题变化**：`themeIcon()` 返回的图标不会自动更新，主题切换后可重新获取。
