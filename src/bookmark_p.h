// \file bookmark_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "bookmark.h"
#include "shell/shell_pidl.h"

class BookmarkPrivate : public QSharedData
{
public:
    enum BookmarkType
    {
        NullBookmark,
        PidlBookmark,
        FtpBookmark
    };

public:
    BookmarkPrivate();
    BookmarkPrivate(const BookmarkPrivate& other);
    ~BookmarkPrivate();

public:
    QString m_name;

    BookmarkType m_type;

    ShellPidl m_pidl;

    QString m_path;
    QString m_user;
    QString m_password;
};
