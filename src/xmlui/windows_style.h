// \file windows_style.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QtGlobal>

#if !defined(Q_OS_WIN) || (QT_VERSION < 0x040600) || defined(QT_NO_STYLE_WINDOWSVISTA)
#    define XMLUI_NO_STYLE_WINDOWS
#endif

#if !defined(XMLUI_NO_STYLE_WINDOWS)

#    include <QProxyStyle>

namespace XmlUi
{

    /**
     * Modern Qt style for Windows.
     *
     * The style imitates the look of MS Office.
     */
    class WindowsStyle : public QProxyStyle
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         */
        WindowsStyle();

        /**
         * Destructor.
         */
        ~WindowsStyle() override;

    public: // overrides
        void polish(QPalette& palette) override;

        void polish(QWidget* widget) override;
        void unpolish(QWidget* widget) override;

        int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const override;

        QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const override;

        int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData = nullptr) const override;

        void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;
        void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;
        void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const override;

    private:
        bool m_whiteTheme;
        bool m_darkTheme;

        QColor m_colorMenuBorder;
        QColor m_colorMenuBackground;
        QColor m_colorBarBegin;
        QColor m_colorBarEnd;
        QColor m_colorMenuSeparator;
        QColor m_colorToolbarSeparator;
        QColor m_colorToolbarBorder;
        QColor m_colorItemBorder;
        QColor m_colorItemBackground;
        QColor m_colorItemChecked;
        QColor m_colorItemSunken;
        QColor m_colorGroupBoxText;
    };

} // namespace XmlUi

#endif // !defined( XMLUI_NO_STYLE_WINDOWS )
