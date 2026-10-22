// \file plugin_manager.cpp
// \author zhangjianhua
// \date 2026/10/22

#include "plugin/plugin_manager.h"
#include "application.h"
#include "main_window.h"
#include "plugin/plugin_host.h"
#include "shell/shell_folder.h"
#include "shell/shell_item.h"
#include "shell/shell_selection.h"
#include "utils/local_settings.h"
#include "xmlui/builder.h"
#include "xmlui/client.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPluginLoader>

PluginManager* PluginManager::m_instance = nullptr;

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
    m_instance = this;
}

PluginManager::~PluginManager()
{
    shutdown();

    qDeleteAll(m_entries);
    m_entries.clear();

    if (m_instance == this) m_instance = nullptr;
}

QStringList PluginManager::pluginPaths() const
{
    QStringList paths;
    paths.append(systemPluginsPath());
    paths.append(userPluginsPath());
    return paths;
}

QString PluginManager::systemPluginsPath() const
{
    return QCoreApplication::applicationDirPath() + "/plugins";
}

QString PluginManager::userPluginsPath() const
{
    // locateDataFile 对不存在的路径只会创建其父目录（它面向文件），这里
    // 显式补建插件目录，给用户一个明确的插件存放位置。
    QString path = g_application->locateDataFile("plugins");

    QDir().mkpath(path);

    return path;
}

void PluginManager::scan()
{
    QStringList paths = pluginPaths();

    for (int i = 0; i < paths.count(); i++)
        scanDirectory(paths.at(i));
}

void PluginManager::scanDirectory(const QString& path)
{
    QDir dir(path);

    if (!dir.exists()) return;

    foreach (QString fileName, dir.entryList(QStringList("*.dll"), QDir::Files, QDir::Name))
    {
        QString filePath = dir.filePath(fileName);

        auto entry = new PluginEntry();
        entry->info.path = QDir::toNativeSeparators(filePath);

        auto loader = new QPluginLoader(filePath, this);
        entry->loader = loader;

        if (!readMetadata(loader, entry->info))
        {
            // readMetadata 已设置状态与错误原因
        }
        else if (entry->info.apiVersion != QLatin1String(SALADIN_PLUGIN_API_VERSION))
        {
            entry->info.state = PluginIncompatible;
            entry->info.errorString = tr("The plugin requires plugin API version %1, but this version of Saladin provides %2.")
                                          .arg(entry->info.apiVersion, QLatin1String(SALADIN_PLUGIN_API_VERSION));
        }
        else if (isDisabledInSettings(entry->info.id))
        {
            entry->info.state = PluginDisabled;
        }
        else
        {
            entry->loadable = true;
        }

        // 同 id 的插件先到先得：用户目录中的重复项不会覆盖系统目录里的。
        bool duplicate = false;
        foreach (PluginEntry* other, m_entries)
        {
            if (other->loadable && other->info.id == entry->info.id)
            {
                duplicate = true;
                break;
            }
        }

        if (duplicate)
        {
            entry->loadable = false;
            entry->info.state = PluginError;
            entry->info.errorString = tr("The plugin id duplicates an already collected plugin, it was skipped.");
        }

        m_entries.append(entry);
    }
}

bool PluginManager::readMetadata(const QPluginLoader* loader, PluginInfo& info)
{
    QJsonObject root = loader->metaData();
    QJsonObject metaData = root.value("MetaData").toObject();

    // 不是 Qt 插件（或元数据为空），例如用户随手放进来的一般 DLL。
    if (metaData.isEmpty())
    {
        info.state = PluginError;
        info.errorString = tr("Not a valid Saladin plugin (plugin metadata is missing).");
        return false;
    }

    // 是 Qt 插件但接口不符，例如误放的 Qt 自带插件。
    if (root.value("IID").toString() != QLatin1String(SALADIN_PLUGIN_IID))
    {
        info.state = PluginError;
        info.errorString = tr("Not a valid Saladin plugin (the plugin interface id does not match).");
        return false;
    }

    info.id = metaData.value("name").toString();
    info.version = metaData.value("version").toString();
    info.apiVersion = metaData.value("apiVersion").toString();
    info.author = metaData.value("author").toString();
    info.description = metaData.value("description").toString();
    info.uiFile = metaData.value("ui").toString();
    info.name = metaData.value("displayName").toString();

    if (info.name.isEmpty()) info.name = info.id;

    if (info.id.isEmpty())
    {
        info.state = PluginError;
        info.errorString = tr("The plugin metadata is missing the name field.");
        return false;
    }

    info.state = PluginDisabled;
    info.errorString.clear();

    return true;
}

void PluginManager::loadPlugins()
{
    foreach (PluginEntry* entry, m_entries)
    {
        if (!entry->loadable) continue;

        loadEntry(entry);
    }

    // 只要还有可用插件，就注册右键菜单提供者；每次弹出菜单时逐插件询问，
    // 没有贡献项时自然什么也不追加。
    bool anyLoaded = false;

    foreach (PluginEntry* entry, m_entries)
    {
        if (entry->info.state != PluginLoaded) continue;

        anyLoaded = true;

        attachUi(entry);
    }

    if (anyLoaded) setShellContextMenuProvider(&PluginManager::contextMenuProvider);
}

void PluginManager::attachUi(PluginEntry* entry)
{
    if (!g_mainWindow || entry->actions.isEmpty()) return;

    auto client = new XmlUi::Client();

    foreach (QAction* action, entry->actions)
    {
        if (!action) continue;

        QString id = action->objectName();

        // 动作 id 必须以 "<插件id>." 开头，避免与其他插件及宿主动作冲突；
        // 插件作者忘记前缀时自动补上，保证隔离始终成立。
        if (!id.startsWith(entry->info.id + QLatin1Char('.')))
        {
            if (!id.isEmpty())
                qWarning("Plugin '%s': action objectName '%s' is missing the plugin id prefix, it was added automatically",
                         qPrintable(entry->info.id),
                         qPrintable(id));

            id = entry->info.id + QLatin1Char('.') + id;
        }

        if (id == entry->info.id + QLatin1Char('.'))
        {
            qWarning("Plugin '%s': an action has no objectName, it will not appear in the UI", qPrintable(entry->info.id));
            continue;
        }

        client->setAction(id, action);
    }

    // 插件 XML 描述动作的摆放位置；引用的容器 id（如 menuBarTools、
    // stripMain）会被 Builder 合并进宿主的现有菜单与工具条。
    if (!entry->info.uiFile.isEmpty())
    {
        QString uiPath = QFileInfo(entry->info.path).dir().filePath(entry->info.uiFile);

        if (!client->loadXmlUiFile(uiPath)) qWarning("Plugin '%s': cannot load ui file '%s'", qPrintable(entry->info.id), qPrintable(entry->info.uiFile));
    }
    else
    {
        qWarning("Plugin '%s': metadata has no 'ui' file, its actions will not appear in the main window", qPrintable(entry->info.id));
    }

    entry->client = client;

    g_mainWindow->builder()->addClient(client);
}

void PluginManager::loadEntry(PluginEntry* entry)
{
    entry->info.state = PluginError;
    entry->info.errorString.clear();

    if (!entry->loader->load())
    {
        entry->info.errorString = entry->loader->errorString();
        return;
    }

    QObject* instance = entry->loader->instance();

    entry->plugin = qobject_cast<Saladin::Plugin*>(instance);

    if (!entry->plugin)
    {
        entry->info.errorString = tr("Not a valid Saladin plugin (the root object does not implement Saladin::Plugin).");
        return;
    }

    // 每个插件一份宿主实现，插件 id 的设置键命名空间随之隔离，
    // 避免多插件场景下相互覆盖。
    entry->host = new Saladin::PluginHostImpl();
    if (g_mainWindow) entry->host->setPanels(g_mainWindow->pane(0), g_mainWindow->pane(1));
    entry->host->setPluginId(entry->info.id);

    try
    {
        if (!entry->plugin->initialize(entry->host))
        {
            entry->info.errorString = tr("Plugin initialization failed (initialize returned false).");
            entry->plugin = nullptr;
            return;
        }

        entry->actions = entry->plugin->createActions(this);
    }
    catch (const std::exception& e)
    {
        entry->info.errorString = tr("The plugin threw an exception during initialization: %1").arg(QString::fromUtf8(e.what()));
        entry->plugin = nullptr;
        return;
    }
    catch (...)
    {
        entry->info.errorString = tr("The plugin threw an unknown exception during initialization.");
        entry->plugin = nullptr;
        return;
    }

    entry->info.state = PluginLoaded;
}

void PluginManager::shutdown() const
{
    XmlUi::Builder* uiBuilder = g_mainWindow ? g_mainWindow->builder() : nullptr;

    for (qsizetype i = m_entries.count() - 1; i >= 0; i--)
    {
        PluginEntry* entry = m_entries.at(i);

        if (entry->client)
        {
            if (uiBuilder) uiBuilder->removeClient(entry->client);

            delete entry->client;
            entry->client = nullptr;
        }

        if (entry->plugin && entry->info.state == PluginLoaded)
        {
            try
            {
                entry->plugin->shutdown();
            }
            catch (...)
            {
                // 退出阶段插件异常只能吞掉，不能让宿主崩溃。
            }
        }

        delete entry->host;
        entry->host = nullptr;

        entry->info.state = PluginDisabled;
    }

    setShellContextMenuProvider(nullptr);
}

QList<PluginInfo> PluginManager::pluginInfos() const
{
    QList<PluginInfo> result;

    foreach (PluginEntry* entry, m_entries)
        result.append(entry->info);

    return result;
}

void PluginManager::setEnabled(const QString& id, bool enabled)
{
    LocalSettings* settings = g_application->applicationSettings();

    QStringList disabled = settings->value("DisabledPlugins").toStringList();

    if (enabled)
        disabled.removeAll(id);
    else if (!disabled.contains(id))
        disabled.append(id);

    settings->setValue("DisabledPlugins", QVariant::fromValue(disabled));
}

bool PluginManager::isDisabledInSettings(const QString& id)
{
    return g_application->applicationSettings()->value("DisabledPlugins").toStringList().contains(id);
}

Saladin::Plugin* PluginManager::loadedPlugin(const QString& id) const
{
    foreach (PluginEntry* entry, m_entries)
    {
        if (entry->info.id == id && entry->info.state == PluginLoaded && entry->plugin) return entry->plugin;
    }

    return nullptr;
}

QList<QAction*> PluginManager::collectContextMenuActions(const ShellFolder* folder, const QList<ShellItem>& items) const
{
    QList<Saladin::FileInfo> infos;

    foreach (ShellItem item, items)
        infos.append(Saladin::PanelApiImpl::fileInfo(folder, item));

    QList<QAction*> result;

    foreach (PluginEntry* entry, m_entries)
    {
        if (entry->info.state != PluginLoaded || !entry->plugin) continue;

        try
        {
            QList<QAction*> actions = entry->plugin->contextMenuActions(infos);

            foreach (QAction* action, actions)
            {
                if (action && !result.contains(action)) result.append(action);
            }
        }
        catch (...)
        {
            // 插件回调出错时跳过它，不影响其余插件与原生菜单。
        }
    }

    return result;
}

QList<QAction*> PluginManager::contextMenuProvider(const ShellFolder* folder, const QList<ShellItem>& items)
{
    if (!m_instance) return {};

    return m_instance->collectContextMenuActions(folder, items);
}
