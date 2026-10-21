// \file plugin_info.h
// \author zhangjianhua
// \date 2026/10/21

#pragma once

#include <QString>

/**
 * 插件在宿主中的状态。
 */
enum PluginState
{
    PluginLoaded,      ///< 已加载并完成初始化
    PluginDisabled,    ///< 被用户停用，未加载
    PluginIncompatible, ///< API 版本不匹配，拒绝加载
    PluginError        ///< 加载或初始化失败（错误原因见 errorString）
};

/**
 * 插件的描述信息，扫描阶段从 metadata.json 读取，供插件管理页展示。
 */
struct PluginInfo
{
    QString id;            ///< 插件唯一标识（metadata.json 的 "name"）
    QString name;          ///< 显示名（缺省等于 id）
    QString version;       ///< 插件版本
    QString apiVersion;    ///< 所需的插件 API 版本
    QString author;
    QString description;

    QString path;          ///< DLL 绝对路径（原生分隔符）
    QString uiFile;        ///< UI 布局 XML 路径（相对 DLL 目录），可为空

    PluginState state = PluginDisabled;
    QString errorString;   ///< PluginError / PluginIncompatible 时的人类可读原因
};
