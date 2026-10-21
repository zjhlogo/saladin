// \file plugin_manager.h
// \author zhangjianhua
// \date 2026/10/21

#pragma once

#include "plugin/plugin_api.h"
#include "plugin/plugin_info.h"

#include <QList>
#include <QObject>

class QPluginLoader;
class ShellFolder;
class ShellItem;

namespace XmlUi
{
    class Client;
}

namespace Saladin
{
    class PluginHostImpl;
}

/**
 * 一条插件记录：描述信息加上宿主持有的加载产物。
 */
struct PluginEntry
{
    PluginInfo info;

    // 元数据有效、版本兼容、未被停用且不与其他插件 id 重复；
    // 只有这样的条目才会在 loadPlugins() 中真正尝试加载。
    bool loadable = false;

    QPluginLoader* loader = nullptr;
    Saladin::Plugin* plugin = nullptr;
    Saladin::PluginHostImpl* host = nullptr; ///< 每个插件独立的宿主服务实例（携带插件 id 命名空间）

    XmlUi::Client* client = nullptr;      ///< 贡献到主窗口 UI 的客户端，可为空
    QList<QAction*> actions;              ///< createActions 的结果（可能为空）
};

/**
 * 插件框架核心：扫描插件目录、加载/初始化插件、把它们贡献的动作注入
 * 主窗口 UI 与文件右键菜单，并在退出时回放 shutdown。
 *
 * 目录约定：
 * - 系统目录 "<exe 目录>\plugins"（随安装分发，先扫描）；
 * - 用户目录 "<AppData>\Saladin\plugins"（用户随手放置，同 id 时不覆盖系统目录中的插件）。
 *
 * 启用状态持久化在 settings.dat 的 "DisabledPlugins"（QStringList，插件 id），
 * 停用/启用在重启后生效。
 */
class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject* parent);
    ~PluginManager() override;

public:
    /**
     * 枚举两个插件目录并读取元数据（不加载 DLL）。
     * 可在 MainWindow 构造之前调用。
     */
    void scan();

    /**
     * 加载、初始化所有未被停用的插件。必须在 MainWindow 构造完成后调用。
     */
    void loadPlugins();

    /** 退出前回调各插件的 shutdown() 并释放 UI 客户端；不卸载 DLL。 */
    void shutdown();

public:
    /** 全部插件的描述信息（含停用与出错的），供插件管理页展示。 */
    QList<PluginInfo> pluginInfos() const;

    /** 把启用状态写入设置（重启后生效）；持久化随设置对话框的 save 完成。 */
    void setEnabled(const QString& id, bool enabled);

    /** 插件当前在设置中是否被停用。 */
    bool isDisabledInSettings(const QString& id) const;

    /** 已成功加载的插件实例（按插件 id 查询）；未加载或已停用返回 nullptr。 */
    Saladin::Plugin* loadedPlugin(const QString& id) const;

    /** 用户插件目录（不存在时会被创建），供"打开插件目录"使用。 */
    QString userPluginsPath() const;

    /** 系统插件目录。 */
    QString systemPluginsPath() const;

private:
    QStringList pluginPaths() const;

    void scanDirectory(const QString& path);

    bool readMetadata(QPluginLoader* loader, PluginInfo& info) const;

    void loadEntry(PluginEntry* entry);

    void attachUi(PluginEntry* entry);

    QList<QAction*> collectContextMenuActions(ShellFolder* folder, const QList<ShellItem>& items) const;

    static QList<QAction*> contextMenuProvider(ShellFolder* folder, const QList<ShellItem>& items);

private:
    QList<PluginEntry*> m_entries;

    static PluginManager* m_instance;
};
