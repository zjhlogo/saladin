// \file operation_dialog.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QDialog>

class MultiRenameWidget;

class OperationDialog : public QDialog
{
    Q_OBJECT
public:
    enum Flag
    {
        WithName = 0x1,
        WithPattern = 0x2,
        WithLocation = 0x4,
        WithSource = 0x8,
        WithTarget = 0x10,
        WithCheckBox = 0x20,
        WithMultiRename = 0x40,
        CanEditName = 0x80,
        CanRename = 0x100
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    OperationDialog(Flags flags, QWidget* parent);
    ~OperationDialog() override;

public:
    void setPromptPixmap(const QPixmap& pixmap);
    void setPrompt(const QString& text);

    void setName(const QString& text);
    void setLocation(const QString& text);
    void setSource(const QString& text);
    void setTarget(const QString& text);

    QString name() const;
    QString pattern() const;

    void setPatternList(const QStringList& list);
    QStringList patternList() const;

    void setCheckBoxText(const QString& text);

    void setCheckBoxChecked(bool checked);
    bool checkBoxChecked() const;

    void setNames(const QStringList& names);
    const QStringList& names() const { return m_names; }

public: // overrides
    void accept() override;

private slots:
    void rename();

private:
    void expandTo(const QSize& newSize);

private:
    QLabel* m_promptLabel{};
    QLabel* m_promptPixmap{};

    QBoxLayout* m_mainLayout{};

    QLineEdit* m_nameEdit{};
    QComboBox* m_patternComboBox{};
    QLineEdit* m_locationEdit{};
    QLineEdit* m_sourceEdit{};
    QLineEdit* m_targetEdit{};

    QCheckBox* m_checkBox{};

    MultiRenameWidget* m_multiRename{};

    QPushButton* m_renameButton{};

    QStringList m_names;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OperationDialog::Flags)
