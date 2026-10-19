// \file gradient_widget.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QWidget>

namespace XmlUi
{

    /**
     * Widget with styled gradient background.
     *
     * It uses the XmlUi::WindowsStyle if available to draw the gradient.
     * Otherwise it falls back to plain white background.
     */
    class GradientWidget : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         * @param parent The parent widget.
         */
        GradientWidget(QWidget* parent);

        /**
         * Destructor.
         */
        ~GradientWidget() override;
    };

} // namespace XmlUi
