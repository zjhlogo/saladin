// \file about_box.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "about_box.h"
#include "utils/icon_loader.h"

#include <QScreen>

AboutBox::AboutBox(const QString& title, const QString& message, QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(title);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    auto mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(4);
    topLayout->addLayout(mainLayout);

    auto scrollArea = new AboutBoxScrollArea(this);
    mainLayout->addWidget(scrollArea);

    scrollArea->setBackgroundRole(QPalette::Base);

    m_sectionsWidget = new QWidget(scrollArea);
    scrollArea->setWidget(m_sectionsWidget);
    scrollArea->setWidgetResizable(true);

    m_sectionsLayout = new QVBoxLayout(m_sectionsWidget);
    m_sectionsLayout->setContentsMargins(6, 6, 6, 6);
    m_sectionsLayout->setSpacing(6);

    auto messageLabel = new QLabel(message, m_sectionsWidget);
    messageLabel->setWordWrap(true);
    messageLabel->setOpenExternalLinks(true);
    m_sectionsLayout->addWidget(messageLabel);

    m_sectionsLayout->addStretch(1);

    mainLayout->addSpacing(7);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

AboutBox::~AboutBox() = default;

AboutBoxSection* AboutBox::addSection(const QPixmap& pixmap, const QString& message)
{
    auto section = new AboutBoxSection(pixmap, message, m_sectionsWidget);

    m_sectionsLayout->insertWidget(m_sectionsLayout->count() - 1, section);

    return section;
}

AboutBoxSection::AboutBoxSection(const QPixmap& pixmap, const QString& message, QWidget* parent)
    : QFrame(parent)
{
    setFrameStyle(Box | Sunken);

    initialize();

    m_pixmapLabel->setPixmap(pixmap);
    m_messageLabel->setText(message);
}

AboutBoxSection::AboutBoxSection(Qt::WindowFlags flags)
    : QFrame(nullptr, flags)
{
    initialize();
}

void AboutBoxSection::initialize()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);

    m_pixmapLabel = new QLabel(this);
    m_mainLayout->addWidget(m_pixmapLabel, 0, Qt::AlignTop | Qt::AlignLeft);

    m_messageLabel = new QLabel(this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setOpenExternalLinks(true);
    m_mainLayout->addWidget(m_messageLabel, 1, Qt::AlignTop);
}

AboutBoxSection::~AboutBoxSection() = default;

void AboutBoxSection::setPixmap(const QPixmap& pixmap)
{
    m_pixmapLabel->setPixmap(pixmap);
}

void AboutBoxSection::setMessage(const QString& message)
{
    m_messageLabel->setText(message);
}

QPushButton* AboutBoxSection::addButton(const QString& text)
{
    if (!m_buttonsLayout)
    {
        m_buttonsLayout = new QHBoxLayout();
        m_buttonsLayout->setSpacing(6);
        m_mainLayout->addLayout(m_buttonsLayout);
    }

    auto button = new QPushButton(text, this);
    m_buttonsLayout->addWidget(button, 0, Qt::AlignBottom);

    return button;
}

void AboutBoxSection::clearButtons()
{
    if (!m_buttonsLayout) return;

    while (m_buttonsLayout->count() > 0)
        delete m_buttonsLayout->itemAt(0)->widget();

    delete m_buttonsLayout;
    m_buttonsLayout = nullptr;
}

AboutBoxToolSection::AboutBoxToolSection()
    : AboutBoxSection(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    setFrameStyle(Panel | Plain);

    setBackgroundRole(QPalette::ToolTipBase);
    setAutoFillBackground(true);

    m_closeButton = new QToolButton(this);
    m_closeButton->setAutoRaise(true);
    m_closeButton->setIconSize(QSize(16, 16));
    m_closeButton->setIcon(IconLoader::icon("close"));
    m_closeButton->setToolTip(tr("Close"));

    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));

    if (QScreen* screen = QGuiApplication::primaryScreen()) connect(screen, &QScreen::availableGeometryChanged, this, &AboutBoxToolSection::updatePosition);
}

AboutBoxToolSection::~AboutBoxToolSection() = default;

bool AboutBoxToolSection::event(QEvent* e)
{
    if (e->type() == QEvent::LayoutRequest)
    {
        int w = 450;
        int h = heightForWidth(w);
        resize(w, h);

        m_closeButton->adjustSize();
        m_closeButton->move(w - m_closeButton->width() - 2, 2);

        updatePosition();

        show();
    }

    return AboutBoxSection::event(e);
}

void AboutBoxToolSection::updatePosition()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect geometry = screen->geometry();
    QRect available = screen->availableGeometry();

    if (available.left() > geometry.left())
        move(available.left(), available.bottom() - height() + 1);
    else if (available.top() > geometry.top())
        move(available.right() - width() + 1, available.top());
    else
        move(available.right() - width() + 1, available.bottom() - height() + 1);
}

AboutBoxScrollArea::AboutBoxScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
}

AboutBoxScrollArea::~AboutBoxScrollArea() = default;

QSize AboutBoxScrollArea::sizeHint() const
{
    int w = 475;
    int fw = frameWidth();
    return {w, widget()->heightForWidth(w - 2 * fw) + 2 * fw};
}
