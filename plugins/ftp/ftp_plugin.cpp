// \file ftp_plugin.cpp
// \author zhangjianhua
// \date 2026/10/22

#include "ftp_plugin.h"
#include "ftp_dialog.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>
#include <QTranslator>

namespace
{
    // 插件自带图标（icons/icons.qrc，前缀 /ftp/icons）。宿主不持有 ftp 图标，
    // 也不再经 PluginHost::themeIcon() 反向借用宿主资源。
    const QString K_ICON_PATH = QStringLiteral(":/ftp/icons/ftp.svg");

    constexpr int K_ICON_SIZES[] = {16, 22, 32, 48};

    // 与宿主 IconLoader 相同的渲染规则：单色 Material SVG 渲染后用
    // CompositionMode_SourceIn 按目标颜色着色。
    QPixmap renderIcon(int size, const QColor& color)
    {
        QSvgRenderer renderer(K_ICON_PATH);
        if (!renderer.isValid()) return QPixmap();

        QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        renderer.render(&painter, QRectF(0, 0, size, size));
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(image.rect(), color);
        painter.end();

        return QPixmap::fromImage(image);
    }

    // 主题文字色就是应用调色板的 WindowText（ThemeHelper 对明暗主题都写入
    // secondaryText），因此取调色板着色即可与宿主图标颜色保持一致，禁用态
    // 同理。主题切换后需要重新取图标（与 PluginHost::themeIcon 相同的限制）。
    QIcon themeIcon()
    {
        const QPalette palette = QGuiApplication::palette();

        QIcon icon;

        for (int size : K_ICON_SIZES)
        {
            icon.addPixmap(renderIcon(size, palette.color(QPalette::Active, QPalette::WindowText)), QIcon::Normal);
            icon.addPixmap(renderIcon(size, palette.color(QPalette::Disabled, QPalette::WindowText)), QIcon::Disabled);
        }

        return icon;
    }
} // namespace

FtpPlugin::FtpPlugin()
{
}

FtpPlugin::~FtpPlugin()
{
}

bool FtpPlugin::initialize(Saladin::PluginHost* host)
{
    m_host = host;

    // 插件自带的翻译目录（与 DLL 一起部署在 "<exe 目录>/plugins/ftp_plugin"
    // 下，构建时由插件的 CMakeLists 复制，或由 tools/release_translations.py
    // 部署），语言跟随宿主（QLocale::default 由宿主按用户设置设置过）。
    // 加载不到或目录为空时保持英文，与宿主的行为一致。
    loadTranslation();

    return true;
}

void FtpPlugin::shutdown()
{
    if (m_translator)
    {
        QCoreApplication::removeTranslator(m_translator);

        delete m_translator;
        m_translator = nullptr;
    }

    m_host = nullptr;
}

QList<QAction*> FtpPlugin::createActions(QObject* parent)
{
    QList<QAction*> actions;

    QAction* action = new QAction(tr("Connect To FTP..."), parent);
    action->setObjectName("ftp.connect"); // 必须 "<插件id>." 开头
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    action->setIcon(themeIcon());
    connect(action, SIGNAL(triggered()), this, SLOT(connectToFtp()));
    actions.append(action);

    return actions;
}

QList<QAction*> FtpPlugin::contextMenuActions(const QList<Saladin::FileInfo>& items)
{
    Q_UNUSED(items);

    // FTP 插件不注入文件右键菜单。
    return {};
}

void FtpPlugin::connectToFtp()
{
    if (!m_host) return;

    FtpDialog dialog(themeIcon(), m_host->mainWindow());

    if (dialog.exec() != QDialog::Accepted) return;

    m_host->sourcePanel()->setDirectory(dialog.path());
}

void FtpPlugin::loadTranslation()
{
    // 与 DLL 同一个插件文件夹下的 translations/（见插件 CMakeLists 的部署）。
    QString dir = QCoreApplication::applicationDirPath() + "/plugins/ftp_plugin/translations";

    auto translator = new QTranslator(this);

    // 同宿主 Application::loadTranslation：加载失败或目录里没有消息都视为
    // 缺少翻译，保持英文。
    if (!translator->load(QLocale(), "ftp", "_", dir) || translator->isEmpty())
    {
        delete translator;
        return;
    }

    m_translator = translator;

    QCoreApplication::installTranslator(m_translator);
}
