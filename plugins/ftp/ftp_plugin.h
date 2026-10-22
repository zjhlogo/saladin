// \file ftp_plugin.h
// \author zhangjianhua
// \date 2026/10/22

#pragma once

// Saladin FTP 插件 —— 提供"连接到 FTP"入口。
//
// 原宿主内置的 FTP 连接功能（主菜单/工具条动作与连接对话框）封装为插件，
// 通过插件 API 的 sourcePanel()->setDirectory() 跳转到输入的 ftp:// 地址。
// 面板对 ftp:// 路径的浏览仍由宿主的 Shell 集成完成。
//
// 开发指南见仓库 plugins/README.md，API 见 src/plugin/plugin_api.h。

#include "plugin/plugin_api.h"

class QTranslator;

class FtpPlugin : public QObject, public Saladin::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SALADIN_PLUGIN_IID FILE "metadata.json")
    Q_INTERFACES(Saladin::Plugin)

public:
    FtpPlugin();
    ~FtpPlugin() override;

public: // Saladin::Plugin
    bool initialize(Saladin::PluginHost* host) override;

    void shutdown() override;

    QList<QAction*> createActions(QObject* parent) override;

    QList<QAction*> contextMenuActions(const QList<Saladin::FileInfo>& items) override;

private slots:
    void connectToFtp();

private:
    void loadTranslation();

private:
    Saladin::PluginHost* m_host{};

    QTranslator* m_translator{};
};
