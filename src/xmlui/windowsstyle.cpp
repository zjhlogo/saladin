/****************************************************************************
* Simple XML-based UI builder for Qt
* Copyright (C) 2007-2015 Michał Męciński
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*   3. Neither the name of the copyright holder nor the names of the
*      contributors may be used to endorse or promote products derived from
*      this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "windowsstyle.h"

#if !defined( XMLUI_NO_STYLE_WINDOWS )

#include "gradientwidget.h"
#include "toolstrip.h"

#include <QStyleOption>
#include <QPainter>
#include <QLibrary>
#include <QApplication>
#include <QMainWindow>
#include <QAbstractButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QLayout>

using namespace XmlUi;

WindowsStyle::WindowsStyle() : QProxyStyle()
{
}

WindowsStyle::~WindowsStyle()
{
}

static QColor blendColors( const QColor& src, const QColor& dest, double alpha )
{
    double red = alpha * src.red() + ( 1.0 - alpha ) * dest.red();
    double green = alpha * src.green() + ( 1.0 - alpha ) * dest.green();
    double blue = alpha * src.blue() + ( 1.0 - alpha ) * dest.blue();
    return QColor( (int)( red + 0.5 ), (int)( green + 0.5 ), (int)( blue + 0.5 ) );
}

void WindowsStyle::polish( QPalette& palette )
{
    QProxyStyle::polish( palette );

    QColor button = palette.color( QPalette::Button );
    QColor base = palette.color( QPalette::Base );
    QColor text = palette.color( QPalette::Text );
    QColor dark = palette.color( QPalette::Dark );
    QColor light = palette.color( QPalette::Light );
    QColor shadow = palette.color( QPalette::Shadow );
    QColor highlight = palette.color( QPalette::Highlight );

    highlight = QColor::fromHsv( highlight.hue(), 204, 255 ).toRgb();

    m_colorBackground = button;

    m_colorMenuBorder = blendColors( dark, base, 0.55 );
    m_colorMenuBackground = button;
    m_colorSeparator = blendColors( dark, base, 0.4 );
    m_colorItemBorder = highlight;

    m_colorItemBackground = blendColors( highlight, base, 0.4 );
    m_colorItemChecked = blendColors( highlight, light, 0.3 );
    m_colorItemSunken = blendColors( highlight, base, 0.5 );
}

void WindowsStyle::polish( QWidget* widget )
{
    if ( qobject_cast<QMainWindow*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground );

    if ( qobject_cast<GradientWidget*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground );

    QProxyStyle::polish( widget );
}

void WindowsStyle::unpolish( QWidget* widget )
{
    if ( qobject_cast<QMainWindow*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground, false );

    if ( qobject_cast<GradientWidget*>( widget ) )
        widget->setAttribute( Qt::WA_StyledBackground, false );

    if ( qobject_cast<ToolStrip*>( widget ) )
        widget->setPalette( QApplication::palette( widget ) );

    QProxyStyle::unpolish( widget );
}

int WindowsStyle::pixelMetric( PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
{
    switch ( metric ) {
        case PM_MenuPanelWidth:
            return 1;
        case PM_MenuHMargin:
            return 0;
        case PM_MenuVMargin:
            return 1;
        case PM_MenuButtonIndicator:
            return 12;
        default:
            break;
    }

    return QProxyStyle::pixelMetric( metric, option, widget );
}

QSize WindowsStyle::sizeFromContents( ContentsType type, const QStyleOption* option,
    const QSize& contentsSize, const QWidget* widget ) const
{
    switch ( type ) {
        case CT_Menu:
            return contentsSize;

        case CT_MenuItem:
            if ( const QStyleOptionMenuItem* menuItem = qstyleoption_cast<const QStyleOptionMenuItem*>( option ) ) {
                if ( menuItem->menuItemType == QStyleOptionMenuItem::Separator )
                    return QSize( 10, 3 );
                int space = 32 + 16;
                if ( menuItem->text.contains( '\t' ) )
                    space += 12;
                if ( menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem ) {
                    QFontMetrics metrics( menuItem->font );
                    QFont fontBold = menuItem->font;
                    fontBold.setBold( true );
                    QFontMetrics metricsBold( fontBold );
                    space += metricsBold.width( menuItem->text ) - metrics.width( menuItem->text );
                }
                return QSize( contentsSize.width() + space, 22 );
            }
            break;

        case CT_ToolButton:
            if ( const QStyleOptionToolButton* optionToolButton = qstyleoption_cast<const QStyleOptionToolButton*>( option ) ) {
                QSize size = contentsSize + QSize( 7, 6 );
                if ( !( optionToolButton->subControls & SC_ToolButtonMenu ) && ( optionToolButton->features & QStyleOptionToolButton::HasMenu ) )
                    size += QSize( 5, 0 );
                return size;
            }
            break;

        default:
            break;
    }

    return QProxyStyle::sizeFromContents( type, option, contentsSize, widget );
}

void WindowsStyle::drawPrimitive( PrimitiveElement element, const QStyleOption* option,
    QPainter* painter, const QWidget* widget ) const
{
    switch ( element ) {
        case PE_Widget:
            if ( qobject_cast<const QMainWindow*>( widget ) ) {
                QRect rect = option->rect;
                if ( QStatusBar* statusBar = widget->findChild<QStatusBar*>() ) {
                    rect.adjust( 0, 0, 0, -statusBar->height() );
                    painter->setPen( option->palette.light().color() );
                    painter->drawLine( rect.bottomLeft() + QPoint( 0, 1 ),
                        rect.bottomRight() + QPoint( 0, 1 ) );
                }
                painter->fillRect( rect, m_colorBackground );
                return;
            }
            if ( qobject_cast<const GradientWidget*>( widget ) ) {
                painter->fillRect( option->rect, m_colorBackground );
                return;
            }
            break;

        case PE_FrameMenu:
            painter->setPen( m_colorMenuBorder );
            painter->setBrush( Qt::NoBrush );
            painter->drawRect( option->rect.adjusted( 0, 0, -1, -1 ) );

            if ( const QMenu* menu = qobject_cast<const QMenu*>( widget ) ) {
                if ( const QMenuBar* menuBar = qobject_cast<const QMenuBar*>( menu->parent() ) ) {
                    QRect rect = menuBar->actionGeometry( menu->menuAction() );
                    if ( !rect.isEmpty() ) {
                        painter->setPen( m_colorMenuBackground );
                        painter->drawLine( 1, 0, rect.width() - 2, 0 );
                    }
                }
            }
            return;

        default:
            break;
    }

    QProxyStyle::drawPrimitive( element, option, painter, widget );
}

static void drawHighlightFrame( QPainter* painter, const QRect& rect, const QColor& background, const QColor& border )
{
    painter->save();

    QRect frameRect = rect.adjusted( 0, 0, -1, -1 );

    painter->setPen( border );
    painter->setBrush( background );
    painter->drawRect( frameRect );

    painter->restore();
}

void WindowsStyle::drawControl( ControlElement element, const QStyleOption* option,
    QPainter* painter, const QWidget* widget ) const
{
    switch ( element ) {
        case CE_ShapedFrame:
            if ( qobject_cast<const ToolStrip*>( widget ) ) {
                painter->setPen( m_colorSeparator );
                painter->drawLine( ( option->rect.left() + option->rect.right() ) / 2, option->rect.top(),
                    ( option->rect.left() + option->rect.right() ) / 2, option->rect.bottom() );
                return;
            }
            break;

        case CE_MenuEmptyArea:
            painter->fillRect( option->rect, m_colorMenuBackground );
            return;

        case CE_MenuItem: {
            painter->save();
            painter->fillRect( option->rect, m_colorMenuBackground );
            if ( option->state & QStyle::State_Selected && option->state & QStyle::State_Enabled ) {
                painter->setPen( m_colorItemBorder );
                painter->setBrush( m_colorItemBackground );
                QRect rect = option->rect.adjusted( 1, 0, -1, 0 );
                drawHighlightFrame( painter, rect, m_colorItemBackground, m_colorItemBorder );
            }
            if ( const QStyleOptionMenuItem* optionItem = qstyleoption_cast<const QStyleOptionMenuItem*>( option ) ) {
                if ( optionItem->menuItemType == QStyleOptionMenuItem::Separator ) {
                    painter->setPen( m_colorSeparator );
                    painter->drawLine( option->rect.left() + 32, ( option->rect.top() + option->rect.bottom() ) / 2,
                        option->rect.right() - 2, ( option->rect.top() + option->rect.bottom() ) / 2 );
                    painter->restore();
                    return;
                }
                QRect checkRect = option->rect.adjusted( 2, 1, -2, -2 );
                checkRect.setWidth( 20 );
                if ( optionItem->checked && option->state & QStyle::State_Enabled ) {
                    painter->setPen( m_colorItemBorder );
                    if ( option->state & QStyle::State_Selected && option->state & QStyle::State_Enabled )
                        painter->setBrush( m_colorItemSunken );
                    else
                        painter->setBrush( m_colorItemChecked );
                    painter->drawRect( checkRect );
                }
                if ( !optionItem->icon.isNull() ) {
                    QIcon::Mode mode;
                    if ( optionItem->state & State_Enabled )
                        mode = ( optionItem->state & State_Selected ) ? QIcon::Active : QIcon::Normal;
                    else
                        mode = QIcon::Disabled;
                    QIcon::State state = optionItem->checked ? QIcon::On : QIcon::Off;
                    QPixmap pixmap = optionItem->icon.pixmap( pixelMetric( PM_SmallIconSize, option, widget ), mode, state );
                    QRect rect = pixmap.rect();
                    rect.moveCenter( checkRect.center() );
                    painter->drawPixmap( rect.topLeft(), pixmap );
                } else if ( optionItem->checked ) {
                    QStyleOption optionCheckMark;
                    optionCheckMark.initFrom( widget );
                    optionCheckMark.rect = checkRect;
                    if ( !( option->state & State_Enabled ) )
                        optionCheckMark.palette.setBrush( QPalette::Text, optionCheckMark.palette.brush( QPalette::Disabled, QPalette::Text ) );
                    drawPrimitive( PE_IndicatorMenuCheckMark, &optionCheckMark, painter, widget );
                }
                QFont font = optionItem->font;
                if ( optionItem->menuItemType == QStyleOptionMenuItem::DefaultItem )
                    font.setBold( true );
                painter->setFont( font );
                QRect textRect = option->rect.adjusted( 32, 1, -16, -1 );
                int flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if ( !styleHint( SH_UnderlineShortcut, option, widget ) )
                    flags |= Qt::TextHideMnemonic;
                QString text = optionItem->text;
                int pos = text.indexOf( '\t' );
                if ( pos >= 0 ) {
                    drawItemText( painter, textRect, flags | Qt::AlignRight, option->palette, option->state & State_Enabled,
                        text.mid( pos + 1 ), QPalette::Text );
                    text = text.left( pos );
                }
                drawItemText( painter, textRect, flags, option->palette, option->state & State_Enabled, text, QPalette::Text );
                if ( optionItem->menuItemType == QStyleOptionMenuItem::SubMenu ) {
                    QStyleOption optionArrow;
                    optionArrow.initFrom( widget );
                    optionArrow.rect = option->rect.adjusted( 0, 4, -4, -4 );
                    optionArrow.rect.setLeft( option->rect.right() - 12 );
                    optionArrow.state = option->state & State_Enabled;
                    drawPrimitive( PE_IndicatorArrowRight, &optionArrow, painter, widget );
                }
            }
            painter->restore();
            return;
        }

        case CE_Splitter:
            if ( qobject_cast<const QMainWindow*>( widget->window() ) )
                return;
            break;

        default:
            break;
    }

    QProxyStyle::drawControl( element, option, painter, widget );
}

void WindowsStyle::drawComplexControl( ComplexControl control, const QStyleOptionComplex* option,
    QPainter* painter, const QWidget* widget ) const
{
    switch ( control ) {
        case CC_ToolButton:
            if ( option->state & State_AutoRaise ) {
                if ( const QStyleOptionToolButton* optionToolButton = qstyleoption_cast<const QStyleOptionToolButton*>( option ) ) {
                    QRect buttonRect = subControlRect( control, option, SC_ToolButton, widget );
                    QStyle::State buttonState = option->state & ~State_Sunken;
                    if ( option->state & State_Sunken ) {
                        if ( optionToolButton->activeSubControls & SC_ToolButton )
                            buttonState |= State_Sunken;
                        else if ( optionToolButton->activeSubControls & SC_ToolButtonMenu )
                            buttonState |= State_MouseOver;
                    }
                    bool selected = buttonState & State_MouseOver && option->state & State_Enabled;
                    bool checked = buttonState & State_On;
                    bool sunken = buttonState & State_Sunken;
                    if ( selected || checked || sunken ) {
                        if ( sunken || selected && checked )
                            drawHighlightFrame( painter, buttonRect, m_colorItemSunken, m_colorItemBorder );
                        else if ( checked )
                            drawHighlightFrame( painter, buttonRect, m_colorItemChecked, m_colorItemBorder );
                        else
                            drawHighlightFrame( painter, buttonRect, m_colorItemBackground, m_colorItemBorder );
                    }
                    QStyleOptionToolButton optionLabel = *optionToolButton;
                    optionLabel.state = buttonState;
                    int fw = pixelMetric( PM_DefaultFrameWidth, option, widget );
                    optionLabel.rect = buttonRect.adjusted( fw, fw, -fw, -fw );
                    drawControl( CE_ToolButtonLabel, &optionLabel, painter, widget );
                    if ( optionToolButton->subControls & SC_ToolButtonMenu ) {
                        QRect menuRect = subControlRect( control, option, SC_ToolButtonMenu, widget );
                        menuRect.adjust( -1, 0, 0, 0 );
                        if ( sunken || optionToolButton->state & State_Sunken && optionToolButton->activeSubControls & SC_ToolButtonMenu )
                            drawHighlightFrame( painter, menuRect, m_colorItemSunken, m_colorItemBorder );
                        else if ( selected )
                            drawHighlightFrame( painter, menuRect, m_colorItemBackground, m_colorItemBorder );
                        QStyleOptionToolButton optionArrow = *optionToolButton;
                        optionArrow.rect = menuRect.adjusted( 2, 3, -1, -3 );
                        drawPrimitive( PE_IndicatorArrowDown, &optionArrow, painter, widget );
                    } else if ( optionToolButton->features & QStyleOptionToolButton::HasMenu ) {
                        int size = pixelMetric( PM_MenuButtonIndicator, option, widget );
                        QRect rect = optionToolButton->rect;
                        QStyleOptionToolButton optionArrow = *optionToolButton;
                        optionArrow.rect = QRect( rect.right() + 4 - size, rect.height() - size + 4, size - 5, size - 5 );
                        drawPrimitive( PE_IndicatorArrowDown, &optionArrow, painter, widget );
                    }
                    return;
            }
            break;
        }

        default:
            break;
    }

    QProxyStyle::drawComplexControl( control, option, painter, widget );
}

#endif // !defined( XMLUI_NO_STYLE_WINDOWS )
