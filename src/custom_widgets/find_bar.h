// \file find_bar.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QTextDocument>
#include <QWidget>

class QComboBox;
class QToolButton;
class QCheckBox;
class QLabel;

/**
 * Bar for finding text.
 */
class FindBar : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent The parent widget.
     */
    FindBar(QWidget* parent);

    /**
     * Destructor.
     */
    ~FindBar() override;

public:
    /**
     * Return the text to find.
     */
    QString text() const;

    void setTextList(const QStringList& list);
    QStringList textList() const;

    /**
     * Set the find flags.
     */
    void setFlags(QTextDocument::FindFlags flags);

    /**
     * Return the find flags.
     */
    QTextDocument::FindFlags flags() const;

    /**
     * Return @c true if find is enabled.
     */
    bool isFindEnabled() const { return m_enabled; }

    /**
     * Show or hide the "Text not found" message.
     */
    void showWarning(bool on);

    /**
     * Set the bound widget to which up/down key events are forwarded.
     */
    void setBoundWidget(QWidget* widget);

    /**
     * Select all text in the edit box.
     */
    void selectAll();

signals:
    /**
     * Emitted when the find text is updated.
     */
    void find(const QString& text);

    /**
     * Emitted when the Find Next button is clicked.
     */
    void findNext();

    /**
     * Emitted when the Find Previous button is clicked.
     */
    void findPrevious();

    /**
     * Emitted when find is enabled or disabled.
     */
    void findEnabled(bool enabled);

protected: // overrides
    void hideEvent(QHideEvent* e) override;

    bool eventFilter(QObject* obj, QEvent* e) override;

private slots:
    void textChanged(const QString& text);

    void caseToggled();

    void loadIcons();

private:
    QToolButton* m_closeButton{};

    QComboBox* m_comboBox{};

    QToolButton* m_previousButton{};
    QToolButton* m_nextButton{};

    QCheckBox* m_caseCheckBox{};

    QLabel* m_warningPixmap{};
    QLabel* m_warningLabel{};

    QWidget* m_boundWidget{};

    bool m_enabled{};
};
