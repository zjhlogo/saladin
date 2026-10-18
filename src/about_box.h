// \file about_box.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

class AboutBoxSection;

class AboutBox : public QDialog
{
    Q_OBJECT
public:
    AboutBox(const QString& title, const QString& message, QWidget* parent);
    ~AboutBox();

public:
    AboutBoxSection* addSection(const QPixmap& pixmap, const QString& message);

private:
    QWidget* m_sectionsWidget;
    QBoxLayout* m_sectionsLayout;
};

class AboutBoxSection : public QFrame
{
    Q_OBJECT
public:
    AboutBoxSection(const QPixmap& pixmap, const QString& message, QWidget* parent);
    ~AboutBoxSection();

protected:
    AboutBoxSection(Qt::WindowFlags flags);

public:
    void setPixmap(const QPixmap& pixmap);
    void setMessage(const QString& message);

    QPushButton* addButton(const QString& text);

    void clearButtons();

private:
    void initialize();

private:
    QBoxLayout* m_mainLayout{};
    QBoxLayout* m_buttonsLayout{};

    QLabel* m_pixmapLabel{};
    QLabel* m_messageLabel{};
};

class AboutBoxToolSection : public AboutBoxSection
{
    Q_OBJECT
public:
    AboutBoxToolSection();
    ~AboutBoxToolSection();

public: // overrides
    bool event(QEvent* e);

private slots:
    void updatePosition();

private:
    QToolButton* m_closeButton;
};

class AboutBoxScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    AboutBoxScrollArea(QWidget* parent);
    ~AboutBoxScrollArea();

public: // overrides
    QSize sizeHint() const;
};
