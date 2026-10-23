// \file zip_packer.h
// \author zhangjianhua
// \date 2026/10/23

#pragma once

#include <QString>
#include <QStringList>

class QWidget;

/**
 * ZIP 压缩包的创建与填充。
 *
 * 空压缩包直接写 ZIP 的“中央目录结束记录”（与宿主原实现相同）；把文件放进
 * 压缩包则通过 Windows Shell 的 ZIP 文件夹处理器完成——先取得压缩包文件夹的
 * IDropTarget，再把选中项构造成 IDataObject 投递进去。这样压缩、进度显示和
 * 重名冲突提示都由 Explorer 负责，和原来的拖放实现完全一致。
 */
namespace Pack
{
    /** 创建（或清空）一个空的 ZIP 文件；失败返回 false。 */
    bool writeEmptyZip(const QString& filePath);

    /**
     * 把 sourceFolderPath 下的 itemNames 复制进 zipPath 指向的压缩包。
     *
     * owner 用作 Shell 进度/冲突对话框的父窗口，可为 nullptr。失败返回 false。
     */
    bool addToZip(QWidget* owner, const QString& sourceFolderPath, const QStringList& itemNames, const QString& zipPath);
} // namespace Pack
