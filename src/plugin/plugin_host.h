// \file plugin_host.h
// \author zhangjianhua
// \date 2026/10/21

#pragma once

#include "plugin/plugin_api.h"

class PanelWidget;
class ShellItem;
class ShellFolder;

namespace Saladin
{

    /**
     * 单个面板（左栏或右栏）的 PanelApi 实现，包装 PanelWidget。
     */
    class PanelApiImpl : public PanelApi
    {
    public:
        PanelApiImpl() : m_panel(nullptr) { }
        explicit PanelApiImpl(PanelWidget* panel);

    public: // PanelApi
        bool isActive() const override;
        QString folderPath() const override;
        QList<FileInfo> items() const override;
        QList<FileInfo> selectedItems() const override;
        FileInfo currentItem() const override;
        bool setDirectory(const QString& path) override;
        void refresh() override;

    public:
        void setPanel(PanelWidget* panel) { m_panel = panel; }

        static FileInfo fileInfo(const ShellFolder* folder, const ShellItem& item);

    private:
        ShellFolder* folder() const;

    private:
        PanelWidget* m_panel;
    };

    /**
     * PluginHost 的宿主端实现，把面板、设置与图标服务暴露给插件。
     */
    class PluginHostImpl : public PluginHost
    {
    public:
        PluginHostImpl();

    public: // PluginHost
        QString saladinVersion() const override;
        QWidget* mainWindow() const override;
        PanelApi* sourcePanel() const override;
        PanelApi* targetPanel() const override;
        QIcon themeIcon(const QString& name) const override;
        QVariant settingsValue(const QString& key, const QVariant& defaultValue = QVariant()) const override;
        void setSettingsValue(const QString& key, const QVariant& value) override;

    public:
        // 插件 id，用于设置键的命名空间；加载每个插件前设置。
        void setPluginId(const QString& id) { m_pluginId = id; }

        // 在 MainWindow 构造完成后把两个面板实例交给宿主实现。
        void setPanels(PanelWidget* leftPane, PanelWidget* rightPane);

    private:
        // 面板 API 对插件而言是可变视图；接口方法声明为 const 仅为语义清晰。
        mutable PanelApiImpl m_pane[2];
        QString m_pluginId;
    };

} // namespace Saladin
