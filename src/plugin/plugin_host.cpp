// \file plugin_host.cpp
// \author zhangjianhua
// \date 2026/10/21

#include "plugin/plugin_host.h"
#include "application.h"
#include "main_window.h"
#include "panel_widget.h"
#include "shell/shell_folder.h"
#include "shell/shell_item.h"
#include "utils/icon_loader.h"
#include "utils/local_settings.h"

#include <QDir>

namespace Saladin
{

    PanelApiImpl::PanelApiImpl(PanelWidget* panel)
        : m_panel(panel)
    {
    }

    bool PanelApiImpl::isActive() const
    {
        return ::g_mainWindow && ::g_mainWindow->sourcePane() == m_panel;
    }

    ShellFolder* PanelApiImpl::folder() const
    {
        return m_panel ? m_panel->getFolder() : nullptr;
    }

    QString PanelApiImpl::folderPath() const
    {
        ShellFolder* f = folder();
        return f ? f->path() : QString();
    }

    QList<FileInfo> PanelApiImpl::items() const
    {
        QList<FileInfo> result;

        ShellFolder* f = folder();
        if (!f) return result;

        foreach (ShellItem item, m_panel->items())
            result.append(fileInfo(f, item));

        return result;
    }

    QList<FileInfo> PanelApiImpl::selectedItems() const
    {
        QList<FileInfo> result;

        ShellFolder* f = folder();
        if (!f) return result;

        foreach (ShellItem item, m_panel->selectedItems())
            result.append(fileInfo(f, item));

        return result;
    }

    FileInfo PanelApiImpl::currentItem() const
    {
        ShellFolder* f = folder();
        if (!f) return FileInfo();

        ShellItem item = m_panel->currentItem();
        if (!item.isValid()) return FileInfo();

        return fileInfo(f, item);
    }

    bool PanelApiImpl::setDirectory(const QString& path)
    {
        if (!m_panel) return false;

        m_panel->setDirectory(path);

        ShellFolder* f = folder();

        // ShellFolder::path() 的分隔符和大小写不保证与入参一致，按规范化
        // 结果判断跳转是否生效（无效路径时面板保持原目录）。
        QString target = QDir::cleanPath(path);
        QString current = QDir::cleanPath(f ? f->path() : QString());

        return !target.isEmpty() && target.compare(current, Qt::CaseInsensitive) == 0;
    }

    void PanelApiImpl::refresh()
    {
        if (m_panel) m_panel->refresh();
    }

    FileInfo PanelApiImpl::fileInfo(const ShellFolder* folder, const ShellItem& item)
    {
        FileInfo info;

        info.name = item.name();
        info.path = folder->itemPath(item);
        info.path.replace(QLatin1Char('\\'), QLatin1Char('/'));
        info.size = item.size();
        info.lastModified = item.lastModified();

        ShellItem::Attributes attributes = item.attributes();
        info.directory = attributes.testFlag(ShellItem::Directory);
        info.hidden = attributes.testFlag(ShellItem::Hidden);
        info.readOnly = attributes.testFlag(ShellItem::ReadOnly);

        return info;
    }

    PluginHostImpl::PluginHostImpl()
        : m_pluginId("unknown")
    {
        // 面板归属在 MainWindow 构造完成后由 PluginManager 指定。
    }

    void PluginHostImpl::setPanels(PanelWidget* leftPane, PanelWidget* rightPane)
    {
        m_pane[0].setPanel(leftPane);
        m_pane[1].setPanel(rightPane);
    }

    QString PluginHostImpl::saladinVersion() const
    {
        return g_application->version();
    }

    QWidget* PluginHostImpl::mainWindow() const
    {
        return ::g_mainWindow;
    }

    PanelApi* PluginHostImpl::sourcePanel() const
    {
        int index = 0;

        if (::g_mainWindow && ::g_mainWindow->sourcePane() && ::g_mainWindow->sourcePane()->getPanelLocation() != PanelWidget::LeftPane) index = 1;

        return &m_pane[index];
    }

    PanelApi* PluginHostImpl::targetPanel() const
    {
        int index = 1;

        if (::g_mainWindow && ::g_mainWindow->sourcePane() && ::g_mainWindow->sourcePane()->getPanelLocation() != PanelWidget::LeftPane) index = 0;

        return &m_pane[index];
    }

    QIcon PluginHostImpl::themeIcon(const QString& name) const
    {
        return IconLoader::icon(name);
    }

    QVariant PluginHostImpl::settingsValue(const QString& key, const QVariant& defaultValue) const
    {
        return g_application->applicationSettings()->value("Plugin." + m_pluginId + "." + key, defaultValue);
    }

    void PluginHostImpl::setSettingsValue(const QString& key, const QVariant& value)
    {
        g_application->applicationSettings()->setValue("Plugin." + m_pluginId + "." + key, value);
    }

} // namespace Saladin
