// \file binary_view.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "viewer/view.h"

class TextEdit;
class BinaryLoader;

class BinaryView : public View
{
    Q_OBJECT
public:
    BinaryView(QObject* parent, QWidget* parentWidget);
    ~BinaryView() override;

public: // overrides
    Type type() const override;

    void load() override;

    void storeSettings() override;

private slots:
    void updateActions();

    void copy();
    void selectAll();

    void toggleHexMode();

    void contextMenuRequested(const QPoint& pos);

    void loadNextBlock();

    void settingsChanged();

    void loadIcons();

private:
    void initializeSettings();

private:
    TextEdit* m_edit;

    BinaryLoader* m_loader;

    qint64 m_length;

    bool m_hexMode;
};
