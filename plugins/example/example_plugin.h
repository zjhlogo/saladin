// \file example_plugin.h
// \author zhangjianhua
// \date 2026/10/21

#pragma once

// Saladin 示例插件 —— 同时也是新插件的开发模板。
//
// 要点（完整指南见仓库 plugins/README.md）：
// 1. 根对象继承 QObject + Saladin::Plugin，声明 Q_PLUGIN_METADATA / Q_INTERFACES；
// 2. metadata.json 提供 name（唯一 id）、version、apiVersion 等元数据；
// 3. createActions() 返回的动作 objectName 必须以 "<插件id>." 开头，
//    并与 plugin.xml 中 <action id="..."/> 一致，由 Builder 合并进主窗口；
// 4. contextMenuActions() 按当前选中项返回要追加到文件右键菜单的动作；
// 5. createSettingsWidget() 可选，控件会显示在设置对话框的插件页里。

#include "plugin/plugin_api.h"

class QLabel;

class ExamplePlugin : public QObject, public Saladin::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SALADIN_PLUGIN_IID FILE "metadata.json")
    Q_INTERFACES(Saladin::Plugin)

public:
    ExamplePlugin();
    ~ExamplePlugin() override;

public: // Saladin::Plugin
    bool initialize(Saladin::PluginHost* host) override;

    void shutdown() override;

    QList<QAction*> createActions(QObject* parent) override;

    QList<QAction*> contextMenuActions(const QList<Saladin::FileInfo>& items) override;

    QWidget* createSettingsWidget(QWidget* parent) override;

private slots:
    void showSelectionInfo();

private:
    void updateSelectionInfo();

private:
    Saladin::PluginHost* m_host{};

    QAction* m_helloAction{};
    QAction* m_contextAction{};

    QLabel* m_clicksLabel{};

    int m_clicks = 0;
};
