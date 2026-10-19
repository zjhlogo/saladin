// \file bookmark.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QSharedData>

class BookmarkPrivate;
class ShellFolder;

class Bookmark
{
public:
    Bookmark();
    Bookmark(const QString& name, ShellFolder* folder, bool withPassword);
    ~Bookmark();

    Bookmark(const Bookmark& other);
    Bookmark& operator=(const Bookmark& other);

public:
    bool isValid() const;

    QString name() const;
    void setName(const QString& name);

    QString path() const;
    QString user() const;
    QString password() const;

    ShellFolder* createFolder(QWidget* parent) const;

public:
    friend bool operator<(const Bookmark& lhs, const Bookmark& rhs);

    friend QDataStream& operator<<(QDataStream& stream, const Bookmark& bookmark);
    friend QDataStream& operator>>(QDataStream& stream, Bookmark& bookmark);

private:
    QSharedDataPointer<BookmarkPrivate> d;
};
