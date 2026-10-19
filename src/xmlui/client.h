// \file client.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "node_p.h"

#include <QMap>
#include <QString>

namespace XmlUi
{
    class Builder;
}

class QAction;
class QDomElement;

namespace XmlUi
{

    /**
     * The UI client containing actions and layout of menus and toolstrips.
     *
     * This class is a container for actions. It also describes the layout
     * of the toolstrips and context menus containing these actions.
     *
     * One ore more clients can be added to a Builder object connected to
     * a window. The client is automatically removed when it's deleted.
     *
     * A component class can inherit the Client class using multiple
     * inheritance, set up the actions and titles and load the layout from
     * an XML file.
     */
    class Client
    {
    public:
        /**
         * Default constructor.
         */
        Client();

        /**
         * Destructor.
         */
        virtual ~Client();

    public:
        /**
         * Add an action to the collection.
         * @param id The identifier of the action.
         * @param action The action to add.
         */
        void setAction(const QString& id, QAction* action);

        /**
         * Find an action with the given identifier.
         * @param id The identifier of the action.
         * @return The action or @c nullptr if it wasn't found.
         */
        QAction* action(const QString& id) const;

        /**
         * Return all actions contained in the client.
         */
        QList<QAction*> actions() const;

        /**
         * Set the title of a menu or section.
         * @param id The identifier of the menu or section.
         * @param title The user-visible name of the element.
         */
        void setTitle(const QString& id, const QString& title);

        /**
         * Return the title of a menu or section.
         * @param id The identifier of the menu or section.
         * @return The user-visible name of the element.
         */
        QString title(const QString& id) const;

        /**
         * Assign a popup menu to the given action.
         * @param actionId The identifier of the action.
         * @param menuId The identifier of the menu.
         * @param defaultId The identifier of the default action in the menu.
         */
        void setPopupMenu(const QString& actionId, const QString& menuId, const QString& defaultId);

        /**
         * Return popup menu for the given action.
         * @param actionId The identifier of the action.
         * @return The identifier of the menu.
         */
        QString popupMenu(const QString& actionId);

        /**
         * Set the default action in the contextual menu.
         * @param menuId The identifier of the menu.
         * @param defaultId The identifier of the default action in the menu.
         */
        void setDefaultMenuAction(const QString& menuId, const QString& defaultId);

        /**
         * Return the default action in the popup or contextual menu.
         * @param menuId The identifier of the menu.
         * @return The identifier of the default action in the menu.
         */
        QString defaultMenuAction(const QString& menuId);

        /**
         * Load the UI layout from the given XML file.
         * @param path The path of the file.
         * @return @c true if the file was loaded, @c false otherwise.
         */
        bool loadXmlUiFile(const QString& path);

        /**
         * Return the associated UI builder.
         */
        Builder* builder() const { return m_builder; }

    private:
        Node createNode(const QDomElement& element);

        NodeType typeFromTag(const QString& tag);

    private:
        QMap<QString, QAction*> m_actions;

        QMap<QString, QString> m_titles;

        QMap<QString, QString> m_menus;
        QMap<QString, QString> m_defaultActions;

        Node m_rootNode;

        Builder* m_builder;

        friend class Builder;
    };

} // namespace XmlUi
