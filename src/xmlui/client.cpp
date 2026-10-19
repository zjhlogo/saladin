// \file client.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "client.h"
#include "builder.h"

#include <QAction>
#include <QDomDocument>
#include <QFile>

using namespace XmlUi;

Client::Client()
    : m_builder(nullptr)
{
}

Client::~Client()
{
    if (m_builder) m_builder->removeClient(this);
}

void Client::setAction(const QString& id, QAction* action)
{
    m_actions.insert(id, action);
}

QAction* Client::action(const QString& id) const
{
    return m_actions.value(id, nullptr);
}

QList<QAction*> Client::actions() const
{
    return m_actions.values();
}

void Client::setTitle(const QString& id, const QString& title)
{
    m_titles.insert(id, title);
}

QString Client::title(const QString& id) const
{
    return m_titles.value(id, QString());
}

void Client::setPopupMenu(const QString& actionId, const QString& menuId, const QString& defaultId)
{
    m_menus.insert(actionId, menuId);
    m_defaultActions.insert(menuId, defaultId);

    QObject::connect(action(actionId), SIGNAL(triggered()), action(defaultId), SIGNAL(triggered()));
}

QString Client::popupMenu(const QString& actionId)
{
    return m_menus.value(actionId, QString());
}

void Client::setDefaultMenuAction(const QString& menuId, const QString& defaultId)
{
    m_defaultActions.insert(menuId, defaultId);
}

QString Client::defaultMenuAction(const QString& menuId)
{
    return m_defaultActions.value(menuId, QString());
}

bool Client::loadXmlUiFile(const QString& path)
{
    QDomDocument document;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("XmlUi::Client::loadXmlUiFile: Cannot open file");
        return false;
    }

    QString error;
    int line;
    int column;
    if (!document.setContent(&file, false, &error, &line, &column))
    {
        qWarning("XmlUi::Client::loadXmlUiFile: Cannot parse file");
        qWarning("(%s, line: %d, column: %d)", qPrintable(error), line, column);
        return false;
    }

    m_rootNode = createNode(document.documentElement());

    if (m_rootNode.type() != Root)
    {
        qWarning("XmlUi::Client::loadXmlUiFile: Incorrect root element type");
        return false;
    }

    return true;
}

Node Client::createNode(const QDomElement& element)
{
    Node node;

    node.setType(typeFromTag(element.tagName()));
    node.setId(element.attribute("id", QString()));

    if (node.type() == Section && element.attribute("uniform") == "1") node.setUniform(true);

    for (QDomElement child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement())
        node.addChild(createNode(child));

    return node;
}

NodeType Client::typeFromTag(const QString& tag)
{
    if (tag == QLatin1String("xmlui")) return Root;
    if (tag == QLatin1String("strip")) return Strip;
    if (tag == QLatin1String("section")) return Section;
    if (tag == QLatin1String("grid")) return Grid;
    if (tag == QLatin1String("row")) return Row;
    if (tag == QLatin1String("menu")) return Menu;
    if (tag == QLatin1String("group")) return Group;
    if (tag == QLatin1String("action")) return Action;
    if (tag == QLatin1String("separator")) return Separator;
    if (tag == QLatin1String("merge")) return Merge;

    qWarning("XmlUi::Client::loadXmlUiFile: Invalid tag '%s'", qPrintable(tag));
    return Unknown;
}
