// \file view.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_pidl.h"
#include "xmlui/client.h"

#include <QObject>

class View : public QObject, public XmlUi::Client
{
    Q_OBJECT
public:
    explicit View(QObject* parent);
    ~View() override;

public:
    enum Type
    {
        Auto,
        Text,
        Binary,
        Image
    };

public:
    QWidget* mainWidget() const { return m_mainWidget; }

    void setPidl(const ShellPidl& pidl);
    const ShellPidl& pidl() const { return m_pidl; }

    void setFormat(const QByteArray& format);
    const QByteArray& format() const { return m_format; }

    const QString& status() const { return m_status; }

    static View* createView(Type type, QObject* parent, QWidget* parentWidget);

public:
    virtual Type type() const = 0;

    virtual void load() = 0;

    virtual void storeSettings() = 0;

    virtual void setFullScreen(bool on);

    // Re-applies the translated texts to the actions and menus owned by the
    // view. Views are QObjects, so they do not receive LanguageChange and are
    // retranslated by their ViewerWindow.
    virtual void retranslateUi();

signals:
    void statusChanged(const QString& status);

protected:
    void setMainWidget(QWidget* widget);

    void setStatus(const QString& status);

private:
    QWidget* m_mainWidget;

    ShellPidl m_pidl;
    QByteArray m_format;

    QString m_status;
};

