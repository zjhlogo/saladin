// \file pack_dialog.h
// \author zhangjianhua
// \date 2026/10/23

#pragma once

#include <QDialog>
#include <QIcon>

class QCheckBox;
class QLineEdit;

/**
 * “打包为 ZIP”对话框：压缩包名称、只读的目标目录和“覆盖时不提示”选项。
 *
 * 与宿主的 OperationDialog 保持同样的信息布局，但插件不链接 saladin.exe，
 * 因此这里自带一个精简实现。
 */
class PackDialog : public QDialog
{
    Q_OBJECT

public:
    PackDialog(const QIcon& icon, const QString& prompt, const QString& defaultName, const QString& target, QWidget* parent);

public:
    QString name() const;

    bool overwrite() const;

public: // overrides
    void accept() override;

private:
    QLineEdit* m_nameEdit{};
    QCheckBox* m_overwriteCheckBox{};
};
