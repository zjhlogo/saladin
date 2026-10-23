// \file guide_dialog.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "guide_dialog.h"
#include "application.h"
#include "utils/icon_loader.h"
#include "utils/theme_helper.h"
#include "xmlui/builder.h"
#include "xmlui/tool_strip.h"

namespace
{
    // The guide pages reference 16 px PNG icons ("../../icons/name-16.png")
    // that were replaced by the Material Symbols SVG set. Serving the theme
    // tinted Material icon keeps the inline images readable on both color
    // schemes (a missing image renders as a light page glyph, which would
    // glare on the dark theme).
    class GuideDocument : public QTextDocument
    {
    public:
        explicit GuideDocument(QObject* parent) : QTextDocument(parent) {}

    protected:
        QVariant loadResource(int type, const QUrl& url) override
        {
            if (type == QTextDocument::ImageResource)
            {
                const QString fileName = QFileInfo(url.toLocalFile()).fileName();

                if (fileName.endsWith(QStringLiteral("-16.png")))
                {
                    const QPixmap pixmap = IconLoader::pixmap(fileName.chopped(7), 16);

                    if (!pixmap.isNull())
                    {
                        QImage image = pixmap.toImage();
                        image.setDevicePixelRatio(pixmap.devicePixelRatio());
                        return image;
                    }
                }
            }

            return QTextDocument::loadResource(type, url);
        }
    };

    QString readGuideResource(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return {};

        return QString::fromUtf8(file.readAll());
    }
} // namespace

GuideDialog::GuideDialog(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Quick Guide"));

    auto action = new QAction(tr("Back"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(goBack()));
    setAction("goBack", action);

    action = new QAction(tr("Forward"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(goForward()));
    setAction("goForward", action);

    action = new QAction(tr("Home"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(goHome()));
    setAction("goHome", action);

    loadXmlUiFile(":/resources/guide_dialog.xml");

    auto builder = new XmlUi::Builder(this);
    builder->addClient(this);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    auto mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(4);
    topLayout->addLayout(mainLayout);

    auto strip = new XmlUi::ToolStrip(this);
    builder->registerToolStrip("stripGuide", strip);
    mainLayout->addWidget(strip);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenExternalLinks(true);
    m_browser->setDocument(new GuideDocument(m_browser));
    mainLayout->addWidget(m_browser, 1);

    connect(m_browser, SIGNAL(historyChanged()), this, SLOT(updateActions()));

    mainLayout->addSpacing(7);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    loadIcons();

    connect(g_application, SIGNAL(themeChanged()), this, SLOT(loadIcons()));
    connect(g_application, SIGNAL(themeChanged()), this, SLOT(updateGuideStyle()));

    updateActions();

    updateGuideStyle();

    goHome();
}

GuideDialog::~GuideDialog() = default;

void GuideDialog::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);

    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void GuideDialog::retranslateUi()
{
    setWindowTitle(tr("Quick Guide"));

    action("goBack")->setText(tr("Back"));
    action("goForward")->setText(tr("Forward"));
    action("goHome")->setText(tr("Home"));

    m_okButton->setText(tr("&OK"));

    // The guide pages are localized in the resource system; reload the home
    // page so the content matches the new interface language as well.
    goHome();
}

void GuideDialog::goBack()
{
    m_browser->backward();
}

void GuideDialog::goForward()
{
    m_browser->forward();
}

void GuideDialog::goHome()
{
    QString language = QLocale().name();
    QString path = QString(":/guide/%1/index.html").arg(language);

    if (!QFile::exists(path))
    {
        language = language.left(language.indexOf('_'));
        path = QString(":/guide/%1/index.html").arg(language);

        if (!QFile::exists(path)) path = ":/guide/en/index.html";
    }

    m_browser->setSource(QUrl::fromLocalFile(path));
}

void GuideDialog::updateActions()
{
    action("goBack")->setEnabled(m_browser->isBackwardAvailable());
    action("goForward")->setEnabled(m_browser->isForwardAvailable());
}

void GuideDialog::updateGuideStyle()
{
    // The pages link the light stylesheet; on the dark theme its colors are
    // overridden by the dark sheet, while the page background and the default
    // text color follow the theme through the QTextBrowser rules that
    // saladin.qss contributes to the stylesheet.
    const bool dark = (ThemeHelper::getAppTheme() == ThemeHelper::AppTheme::Dark);

    m_browser->document()->setDefaultStyleSheet(dark ? readGuideResource(":/guide/style-dark.css") : QString());

    if (!m_browser->source().isEmpty()) m_browser->reload();
}

void GuideDialog::loadIcons()
{
    action("goBack")->setIcon(IconLoader::icon("arrow-left"));
    action("goForward")->setIcon(IconLoader::icon("arrow-right"));
    action("goHome")->setIcon(IconLoader::icon("home"));
}
