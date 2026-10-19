// \file bookmarks_dialog.h
// \author zhangjianhua
// \date 2026/10/18
#pragma once

#include "bookmark.h"
#include "xmlui/client.h"

#include <QDialog>

class QTreeWidget;

class BookmarksDialog : public QDialog, public XmlUi::Client
{
    Q_OBJECT
public:
    explicit BookmarksDialog(QWidget* parent);
    ~BookmarksDialog() override;

public: // overrides
    void accept() override;

private slots:
    void editRename();
    void editDelete();

    void updateActions();

    void doubleClicked(const QModelIndex& index);
    void listContextMenu(const QPoint& pos);

private:
    void populateList();

private:
    QTreeWidget* m_list{};

    QList<Bookmark> m_bookmarks;
};
