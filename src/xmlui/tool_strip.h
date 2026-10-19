// \file tool_strip.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QToolButton>
#include <QWidget>

namespace XmlUi
{
    class ToolStripLayout;
    class ToolStripSectionLayout;
} // namespace XmlUi

class QBoxLayout;
class QGridLayout;

namespace XmlUi
{
    /**
     * Widget containing tool buttons.
     *
     * This widget can be used as a replacement of the menu bar and toolbars.
     * It displays tool buttons of different size layed out and grouped in sections.
     * The tool strip can be populated with actions using the Builder object.
     */
    class ToolStrip : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         * @param parent The parent widget.
         */
        ToolStrip(QWidget* parent);

        /**
         * Destructor.
         */
        ~ToolStrip() override;

    public:
        /**
         * Set the header widget displayed in the left part of the tool strip.
         */
        void setHeaderWidget(QWidget* widget);

        /**
         * Return the header widget.
         */
        QWidget* headerWidget() const;

        /**
         * Add a tool button associated with given action.
         * @param action The action to add.
         */
        void addToolAction(QAction* action);

        /**
         * Add a separator between buttons.
         */
        void addSeparator();

        /**
         * Begin a section containing actions.
         * @param title The title of the section.
         * @param uniform When @t true all buttons have uniform width.
         */
        void beginSection(const QString& title, bool uniform = false);

        /**
         * End the current section.
         */
        void endSection();

        /**
         * Begin a horizontal row of small buttons in a grid layout.
         */
        void beginRow();

        /**
         * End the current row.
         */
        void endRow();

        /**
         * Remove all tool buttons and sections.
         * The header widget and auxiliary actions are not removed.
         */
        void clearToolActions();

        /**
         * Add a tool button associated with given action in the top right corner
         * of the tool strip.
         * @param action The action to add.
         */
        void addAuxiliaryAction(QAction* action);

        /**
         * Remove all auxiliary actions.
         */
        void clearAuxiliaryActions();

        /**
         * Set the margins for tool strip contents.
         */
        void setContentsMargins(int left, int top, int right, int bottom);

        /**
         * Execute the popup menu associated with given action.
         */
        void execMenu(QAction* action);

    protected: // overrides
        void childEvent(QChildEvent* e) override;
        void paintEvent(QPaintEvent* e) override;

    private:
        enum ButtonSize
        {
            SmallButton,
            MediumButton,
            LargeButton
        };

    private:
        QToolButton* createButton(QAction* action, ButtonSize size);

    private:
        ToolStripLayout* m_layout{};
        ToolStripSectionLayout* m_sectionLayout{};
        QBoxLayout* m_rowLayout{};
        QList<QToolButton*> m_toolButtons;
    };

    /**
     * Action with extended properties.
     */
    class ToolStripAction : public QAction
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         * @param text The text of the action.
         * @param parent The parent object.
         */
        ToolStripAction(const QString& text, QObject* parent);

        /**
         * Constructor.
         * @param icon The icon of the action.
         * @param text The text of the action.
         * @param parent The parent object.
         */
        ToolStripAction(const QIcon& icon, const QString& text, QObject* parent);

        /**
         * Destructor.
         */
        ~ToolStripAction() override;

    public:
        /**
         * Set the menu popup mode for the tool button.
         */
        void setPopupMode(QToolButton::ToolButtonPopupMode mode);

        /**
         * Return the menu popup mode for the tool button.
         */
        QToolButton::ToolButtonPopupMode popupMode() const { return m_popupMode; }

    private:
        QToolButton::ToolButtonPopupMode m_popupMode;
    };

} // namespace XmlUi
