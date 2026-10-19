// \file tool_strip_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QLayout>
#include <QToolButton>

namespace XmlUi
{
    class ToolStripLayout : public QLayout
    {
        Q_OBJECT
    public:
        ToolStripLayout(QWidget* parent);
        ~ToolStripLayout() override;

    public:
        void setHeaderWidget(QWidget* widget);
        QWidget* headerWidget() const { return m_headerWidget; }

        void addLayout(ToolStripSectionLayout* layout);

        void addAuxiliaryButton(QToolButton* button);

        void clear();

        void clearAuxiliaryButtons();

        bool isSimpleLayout() const { return m_simpleLayout; }

    public: // overrides
        Qt::Orientations expandingDirections() const override;

        QSize sizeHint() const override;
        QSize minimumSize() const override;
        QSize maximumSize() const override;

        int count() const override;
        QLayoutItem* itemAt(int index) const override;

        void addItem(QLayoutItem* item) override;
        QLayoutItem* takeAt(int index) override;

        void setGeometry(const QRect& rect) override;

        void invalidate() override;

    private:
        void calculateSize();

        ToolStripSectionLayout* layoutAt(int index) const;
        QToolButton* buttonAt(int index) const;

    private:
        QWidget* m_headerWidget;

        QList<QLayoutItem*> m_items;

        QList<QToolButton*> m_auxiliaryButtons;
        QToolButton* m_chevronButton;

        bool m_dirty;
        bool m_simpleLayout;
        QSize m_sizeHint;
        QSize m_minimumSize;
        QSize m_maximumSize;
        int m_auxWidth;
        int m_auxWidthNoChevron;
    };

    class ToolStripSectionLayout : public QLayout
    {
        Q_OBJECT
    public:
        ToolStripSectionLayout(const QString& title, bool uniform);
        ~ToolStripSectionLayout() override;

    public:
        void addLayout(QLayout* layout);

        void drawSection(QPainter* painter, QWidget* widget);

        bool isUniform() const { return m_uniform; }

        void setCollapsed(bool collapsed);
        bool isCollapsed() const { return m_collapsed; }

    public: // overrides
        Qt::Orientations expandingDirections() const override;

        QSize sizeHint() const override;
        QSize minimumSize() const override;
        QSize maximumSize() const override;

        int count() const override;
        QLayoutItem* itemAt(int index) const override;

        void addItem(QLayoutItem* item) override;
        QLayoutItem* takeAt(int index) override;

        void setGeometry(const QRect& rect) override;

        void invalidate() override;

    private:
        void calculateSize();

    private:
        QList<QLayoutItem*> m_items;

        QString m_titleText;
        QRect m_titleRect;

        bool m_uniform;

        QRect m_separatorRect;

        bool m_collapsed;

        bool m_dirty;
        QSize m_sizeHint;
        QSize m_titleSize;
    };

    class ChevronButton : public QToolButton
    {
        Q_OBJECT
    public:
        ChevronButton(QWidget* parent);
        ~ChevronButton() override;

    protected: // overrides
        void paintEvent(QPaintEvent* e) override;
    };

    class ActionButton : public QToolButton
    {
        Q_OBJECT
    public:
        ActionButton(QWidget* parent);
        ~ActionButton() override;

    public:
        void adjustText();

    protected: // overrides
        void paintEvent(QPaintEvent* e) override;
        void actionEvent(QActionEvent* e) override;

    private:
        bool hasMenuBadge() const;
    };

} // namespace XmlUi
