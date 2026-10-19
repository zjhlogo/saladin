// \file view.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "view.h"
#include "viewer/binary_view.h"
#include "viewer/image_view.h"
#include "viewer/text_view.h"

View::View(QObject* parent)
    : QObject(parent)
    , m_mainWidget(nullptr)
{
}

View::~View()
{
    delete m_mainWidget;
}

void View::setMainWidget(QWidget* widget)
{
    m_mainWidget = widget;
}

void View::setPidl(const ShellPidl& pidl)
{
    m_pidl = pidl;
}

void View::setFormat(const QByteArray& format)
{
    m_format = format;
}

void View::setStatus(const QString& status)
{
    if (m_status != status)
    {
        m_status = status;
        emit statusChanged(status);
    }
}

View* View::createView(Type type, QObject* parent, QWidget* parentWidget)
{
    switch (type)
    {
    case Text:
        return new TextView(parent, parentWidget);
    case Binary:
        return new BinaryView(parent, parentWidget);
    case Image:
        return new ImageView(parent, parentWidget);
    default:
        return nullptr;
    }
}

void View::setFullScreen(bool on)
{
    QLayout* layout = mainWidget()->layout();
    if (on)
    {
        layout->setContentsMargins(0, 0, 0, 0);
        if (auto frame = qobject_cast<QFrame*>(layout->itemAt(0)->widget())) frame->setFrameStyle(0);
    }
    else
    {
        layout->setContentsMargins(3, 0, 3, 0);
        if (auto frame = qobject_cast<QFrame*>(layout->itemAt(0)->widget())) frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    }
}
