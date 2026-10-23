// \file pack_plugin.h
// \author zhangjianhua
// \date 2026/10/23

#pragma once

// Saladin 打包插件 —— 把源面板中选中的文件打包为 ZIP 压缩包。
//
// 原宿主内置的“工具 -> 打包”功能（packToZip）移出为插件：插件通过
// PluginHost 读取源面板的选中项与目标面板目录，自己完成空 ZIP 的创建，
// 并借助 Windows Shell 的 ZIP 文件夹处理器把选中项复制进压缩包，行为与
// 原来的拖放实现一致（复用 Explorer 的进度与冲突提示）。
//
// 开发指南见仓库 plugins/README.md，API 见 src/plugin/plugin_api.h。

#include "plugin/plugin_api.h"

class QTranslator;

class PackPlugin : public QObject, public Saladin::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SALADIN_PLUGIN_IID FILE "metadata.json")
    Q_INTERFACES(Saladin::Plugin)

public:
    PackPlugin();
    ~PackPlugin() override;

public: // Saladin::Plugin
    bool initialize(Saladin::PluginHost* host) override;

    void shutdown() override;

    QList<QAction*> createActions(QObject* parent) override;

    QList<QAction*> contextMenuActions(const QList<Saladin::FileInfo>& items) override;

private slots:
    void packToZip();

private:
    void loadTranslation();

private:
    Saladin::PluginHost* m_host{};

    QTranslator* m_translator{};
};
