// \file pack_plugin.cpp
// \author zhangjianhua
// \date 2026/10/23

#include "pack_plugin.h"
#include "pack_dialog.h"
#include "zip_packer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>
#include <QTranslator>

namespace
{
    // 插件自带图标（icons/icons.qrc，前缀 /pack/icons），不再占用宿主资源。
    const QString K_ICON_PATH = QStringLiteral(":/pack/icons/pack.svg");

    constexpr int K_ICON_SIZES[] = {16, 22, 32, 48};

    // 与宿主 IconLoader 相同的着色规则：单色 Material SVG 渲染后用
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

PackPlugin::PackPlugin()
{
}

PackPlugin::~PackPlugin()
{
}

bool PackPlugin::initialize(Saladin::PluginHost* host)
{
    m_host = host;

    // 插件自带翻译目录（与 DLL 一起部署在 "<exe 目录>/plugins/pack_plugin" 下）。
    loadTranslation();

    return true;
}

void PackPlugin::shutdown()
{
    if (m_translator)
    {
        QCoreApplication::removeTranslator(m_translator);

        delete m_translator;
        m_translator = nullptr;
    }

    m_host = nullptr;
}

QList<QAction*> PackPlugin::createActions(QObject* parent)
{
    QList<QAction*> actions;

    QAction* action = new QAction(tr("Pack..."), parent);
    action->setObjectName("pack.toZip"); // 必须 "<插件id>." 开头
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    action->setStatusTip(tr("Pack the selected items into a ZIP archive in the target directory."));
    action->setIcon(themeIcon());
    connect(action, SIGNAL(triggered()), this, SLOT(packToZip()));
    actions.append(action);

    return actions;
}

QList<QAction*> PackPlugin::contextMenuActions(const QList<Saladin::FileInfo>& items)
{
    Q_UNUSED(items);

    // 打包入口只保留在“工具”菜单与工具条中，不注入文件右键菜单。
    return {};
}

void PackPlugin::packToZip()
{
    if (!m_host) return;

    Saladin::PanelApi* sourcePanel = m_host->sourcePanel();
    Saladin::PanelApi* targetPanel = m_host->targetPanel();

    QList<Saladin::FileInfo> items = sourcePanel->selectedItems();
    if (items.isEmpty()) return;

    const QString sourceFolder = sourcePanel->folderPath();
    const QString targetFolder = targetPanel->folderPath();

    if (sourceFolder.isEmpty() || targetFolder.isEmpty()) return;
    if (!QFileInfo(targetFolder).isDir()) return;

    QStringList itemNames;
    foreach (const Saladin::FileInfo& item, items)
    {
        // 虚拟位置（此电脑、控制面板等）没有文件系统路径，无法按路径打包。
        if (item.name.isEmpty() || item.path.isEmpty()) return;
        itemNames.append(item.name);
    }

    QString defaultName;
    if (items.count() == 1)
    {
        defaultName = items.first().name;

        int pos = defaultName.lastIndexOf('.');
        if (pos > 0) defaultName.truncate(pos);
    }
    else
    {
        defaultName = QFileInfo(sourceFolder).fileName();
    }

    defaultName += ".zip";

    QString prompt;
    if (items.count() == 1)
        prompt = tr("Pack <b>%1</b>:").arg(items.first().name.toHtmlEscaped());
    else
        prompt = tr("Pack <b>%1</b> selected items:").arg(items.count());

    PackDialog dialog(themeIcon(), prompt, defaultName, targetFolder, m_host->mainWindow());

    if (dialog.exec() != QDialog::Accepted) return;

    const QString name = dialog.name().trimmed();
    if (name.isEmpty()) return;

    const QString zipPath = QDir(targetFolder).filePath(name);
    const QFileInfo zipInfo(zipPath);

    if (zipInfo.exists() && zipInfo.isDir())
    {
        QMessageBox::warning(m_host->mainWindow(), tr("Pack To Zip"), tr("\"%1\" is a directory.").arg(name));
        return;
    }

    // 与宿主原实现一致：未勾选“覆盖时不提示”时保留已存在的压缩包并继续添加；
    // 否则（或压缩包尚不存在）新建一个空 ZIP。
    if (!zipInfo.exists() || dialog.overwrite())
    {
        if (!Pack::writeEmptyZip(zipPath))
        {
            QMessageBox::warning(m_host->mainWindow(), tr("Pack To Zip"), tr("Could not create \"%1\".").arg(name));
            return;
        }
    }

    if (!Pack::addToZip(m_host->mainWindow(), sourceFolder, itemNames, zipPath))
    {
        QMessageBox::warning(m_host->mainWindow(), tr("Pack To Zip"), tr("Could not pack the selected items into \"%1\".").arg(name));
        return;
    }

    targetPanel->refresh();
}

void PackPlugin::loadTranslation()
{
    // 与 DLL 同一个插件文件夹下的 translations/（见插件 CMakeLists 的部署）。
    QString dir = QCoreApplication::applicationDirPath() + "/plugins/pack_plugin/translations";

    auto translator = new QTranslator(this);

    if (!translator->load(QLocale(), "pack", "_", dir) || translator->isEmpty())
    {
        delete translator;
        return;
    }

    m_translator = translator;

    QCoreApplication::installTranslator(m_translator);
}
