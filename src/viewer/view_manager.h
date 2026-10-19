// \file view_manager.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "viewer/view.h"

#include <QObject>

class ShellPidl;
class ViewManagerPrivate;
class ViewerWindow;
class ViewItem;

class ViewManager : public QObject
{
    Q_OBJECT
public:
    ViewManager();
    ~ViewManager() override;

public:
    void openView(const ShellPidl& pidl);
    void openView(const QList<ShellPidl>& pidls);

    void loadPrevious(ViewerWindow* window);
    void loadNext(ViewerWindow* window);

    void switchViewType(ViewerWindow* window, View::Type type);

private slots:
    void windowDestroyed(QObject* window);

private:
    void loadView(ViewItem& item, View::Type type);

    void updateTitle(ViewItem& item, const QString& name);

    bool checkType(const ShellPidl& pidl, View::Type inType, View::Type& outType, QByteArray& format, QString& name);

private:
    ViewManagerPrivate* d;
};
