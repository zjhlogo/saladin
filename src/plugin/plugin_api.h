// \file plugin_api.h
// \author zhangjianhua
// \date 2026/10/21

#pragma once

// Saladin 插件 API（纯头文件，插件与宿主共享）。
//
// 插件是一个实现 Saladin::Plugin 接口的 Qt 动态库，通过 Q_PLUGIN_METADATA
// 声明元数据（见 plugins/example/metadata.json），由宿主用 QPluginLoader 加载。
//
// 跨边界调用只使用本头文件中的纯虚接口和值类型，插件不链接 saladin.exe：
// 宿主在 initialize() 时传入 PluginHost*，指针仅在 initialize() 成功返回到
// shutdown() 调用之间有效，且所有调用必须发生在 GUI 线程。
//
// 完整的开发指南见仓库 plugins/README.md。

#include <QAction>
#include <QDateTime>
#include <QIcon>
#include <QList>
#include <QString>
#include <QVariant>
#include <QWidget>

// 插件接口标识与 API 版本。插件的 Q_PLUGIN_METADATA 必须使用相同的 IID，
// 且 metadata.json 中的 "apiVersion" 必须与本宏一致，否则宿主拒绝加载。
#define SALADIN_PLUGIN_IID "org.saladin.Plugin"
#define SALADIN_PLUGIN_API_VERSION "1.0"

namespace Saladin
{

    /**
     * 面板中一个条目的快照。
     *
     * path 是宿主按 ShellFolder::itemPath() 解析出的完整路径；虚拟位置
     * （如"此电脑"、控制面板）可能拿不到路径，此时 path 为空字符串。
     */
    struct FileInfo
    {
        QString name;
        QString path;
        quint64 size = 0;
        QDateTime lastModified;
        bool directory = false;
        bool hidden = false;
        bool readOnly = false;
    };

    /**
     * 一个文件面板（左栏或右栏）的只读视图与有限的导航能力。
     *
     * 实例由宿主持有并贯穿整个会话；sourcePanel()/targetPanel() 返回的是
     * 同一对实例，分别指向当前活动（源）面板与另一侧（目标）面板，活动
     * 面板切换后再次调用会返回不同的实例。
     */
    class PanelApi
    {
    public:
        virtual ~PanelApi() { }

    public:
        /** 面板当前是否为活动（源）面板。 */
        virtual bool isActive() const = 0;

        /** 当前目录的路径；面板尚未就绪时返回空字符串。 */
        virtual QString folderPath() const = 0;

        /** 当前目录中的全部条目。 */
        virtual QList<FileInfo> items() const = 0;

        /** 当前选中的条目；无显式选中时为当前焦点条目，可能为空。 */
        virtual QList<FileInfo> selectedItems() const = 0;

        /** 当前焦点条目；可能无效（name 为空）。 */
        virtual FileInfo currentItem() const = 0;

        /** 跳转到指定目录；返回是否成功。 */
        virtual bool setDirectory(const QString& path) = 0;

        /** 强制刷新面板内容。 */
        virtual void refresh() = 0;
    };

    /**
     * 宿主提供给插件的服务入口。
     */
    class PluginHost
    {
    public:
        virtual ~PluginHost() { }

    public:
        /** Saladin 程序版本号，例如 "1.0"。 */
        virtual QString saladinVersion() const = 0;

        /** 主窗口，用作插件对话框的父窗口；不要对它做 Qt 之外的假设。 */
        virtual QWidget* mainWindow() const = 0;

        /** 当前活动（源）面板；随用户切换面板而变化。 */
        virtual PanelApi* sourcePanel() const = 0;

        /** 非活动（目标）面板。 */
        virtual PanelApi* targetPanel() const = 0;

        /**
         * 取一个按当前主题着色的内置 Material 图标（与 IconLoader 同名）。
         * 主题切换后图标不会自动更新，插件应缓存 QIcon 并在需要时重新获取。
         */
        virtual QIcon themeIcon(const QString& name) const = 0;

        /**
         * 读写插件私有设置。键会自动加 "Plugin.<插件id>." 前缀，持久化在
         * 用户 settings.dat 中，与宿主及其他插件互不干扰。
         */
        virtual QVariant settingsValue(const QString& key, const QVariant& defaultValue = QVariant()) const = 0;

        virtual void setSettingsValue(const QString& key, const QVariant& value) = 0;
    };

    /**
     * 插件接口。实现类必须是一个 QObject，并在类声明中写：
     *
     *     Q_OBJECT
     *     Q_PLUGIN_METADATA(IID SALADIN_PLUGIN_IID FILE "metadata.json")
     *     Q_INTERFACES(Saladin::Plugin)
     */
    class Plugin
    {
    public:
        virtual ~Plugin() { }

    public:
        /**
         * 加载后调用一次。返回 false 表示初始化失败，宿主会记为错误状态
         * 并跳过该插件（不卸载 DLL，shutdown 也不会被调用）。
         */
        virtual bool initialize(PluginHost* host) = 0;

        /** 程序退出前调用一次；调用后宿主不再使用任何 PluginHost 指针。 */
        virtual void shutdown() = 0;

        /**
         * 贡献到主窗口菜单/工具条的动作。
         *
         * 每个动作的 objectName 必须是 "<插件id>.<名称>"，并与插件 ui XML
         * （metadata.json 的 "ui" 字段）中的 <action id="..."/> 对应；id 带
         * 前缀是为了避免与其他插件及宿主动作冲突。宿主持有动作的所有权。
         */
        virtual QList<QAction*> createActions(QObject* parent) = 0;

        /**
         * 文件列表右键菜单的追加项。宿主在每次弹出原生 Shell 右键菜单前
         * 用当前选中项调用（空白处右键时列表为空）；返回的动作将按顺序
         * 追加到菜单末尾。禁用态动作显示为灰色。动作的所有权归插件。
         */
        virtual QList<QAction*> contextMenuActions(const QList<FileInfo>& items) = 0;

        /**
         * 可选：插件设置控件，显示在设置对话框插件页选中项下方；返回的
         * 控件由宿主在对话框关闭时销毁。默认没有设置页。
         */
        virtual QWidget* createSettingsWidget(QWidget* parent)
        {
            Q_UNUSED(parent);
            return nullptr;
        }
    };

} // namespace Saladin

Q_DECLARE_INTERFACE(Saladin::Plugin, SALADIN_PLUGIN_IID)
