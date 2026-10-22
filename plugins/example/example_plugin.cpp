// \file example_plugin.cpp
// \author zhangjianhua
// \date 2026/10/21

#include "example_plugin.h"

#include <QLabel>
#include <QMessageBox>

ExamplePlugin::ExamplePlugin()
{
}

ExamplePlugin::~ExamplePlugin()
{
}

bool ExamplePlugin::initialize(Saladin::PluginHost* host)
{
    m_host = host;

    // 演示插件私有设置：计数器跨会话保留，键实际存储为
    // "Plugin.example.ClickCount"。
    m_clicks = m_host->settingsValue("ClickCount").toInt();

    return true;
}

void ExamplePlugin::shutdown()
{
    // 宿主即将销毁 UI；这里释放仍持有的资源（本插件没有），
    // 不要再触碰 m_host 返回的任何指针。
    m_host = nullptr;
}

QList<QAction*> ExamplePlugin::createActions(QObject* parent)
{
    QList<QAction*> actions;

    m_helloAction = new QAction(tr("Example: Selected Files"), parent);
    m_helloAction->setObjectName("example.hello"); // 必须以 "<插件id>." 开头
    m_helloAction->setIcon(m_host->themeIcon("info"));
    m_helloAction->setStatusTip(tr("Show information about the selected files in the source panel."));
    connect(m_helloAction, SIGNAL(triggered()), this, SLOT(showSelectionInfo()));
    actions.append(m_helloAction);

    return actions;
}

QList<QAction*> ExamplePlugin::contextMenuActions(const QList<Saladin::FileInfo>& items)
{
    updateSelectionInfo();

    // 返回空列表时右键菜单不会追加任何内容；空白处右键时 items 也为空。
    if (items.isEmpty()) return {};

    if (!m_contextAction)
    {
        m_contextAction = new QAction(this);
        connect(m_contextAction, SIGNAL(triggered()), this, SLOT(showSelectionInfo()));
    }

    m_contextAction->setText(tr("Example: info about %1 item(s)").arg(items.count()));

    return QList<QAction*>() << m_contextAction;
}

QWidget* ExamplePlugin::createSettingsWidget(QWidget* parent)
{
    // 演示可选的插件设置页；控件在设置对话框关闭时随之销毁，不要缓存指针。
    auto label = new QLabel(parent);
    label->setText(tr("The Example button has been clicked %1 time(s).").arg(m_clicks));

    m_clicksLabel = label;

    return label;
}

void ExamplePlugin::showSelectionInfo()
{
    if (!m_host) return;

    m_clicks++;
    m_host->setSettingsValue("ClickCount", m_clicks);
    if (m_clicksLabel) m_clicksLabel->setText(tr("The Example button has been clicked %1 time(s).").arg(m_clicks));

    QList<Saladin::FileInfo> items = m_host->sourcePanel()->selectedItems();

    if (items.isEmpty())
    {
        QMessageBox::information(m_host->mainWindow(), tr("Example Plugin"), tr("No files are selected in the source panel."));
        return;
    }

    QString message;

    foreach (Saladin::FileInfo item, items)
    {
        QString size = item.directory ? tr("folder") : QString::number(item.size) + " bytes";
        message += QString("<b>%1</b> (%2)<br>%3").arg(item.name.toHtmlEscaped(), size, item.path.toHtmlEscaped()) + "<br>";
    }

    QMessageBox::information(m_host->mainWindow(), tr("Example Plugin"), message);
}

void ExamplePlugin::updateSelectionInfo()
{
    // 右键菜单项每次弹出前都会重新询问，这里预留一个按选中内容更新
    // 状态（如可用性、文本）的位置。
}
