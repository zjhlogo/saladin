// \file drive_strip_manager.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_drive.h"
#include "xmlui/tool_strip.h"

#include <QObject>
#include <functional>

class ShellFolder;
class DriveStripManager : public QObject
{
    Q_OBJECT
public:
    explicit DriveStripManager(QWidget* parent);
    ~DriveStripManager() override;

public:
    ShellComputer* computer() const { return m_computer; }

    ShellDrive driveAt(int index) const;
    ShellDrive driveAt(XmlUi::ToolStrip* strip, const QPoint& pos) const;

    ShellDrive driveFromFolder(ShellFolder* folder) const;

    void registerToolStrip(XmlUi::ToolStrip* strip, QObject* receiver, const std::function<void(int)>& driveSelected);

    void showDrivesMenu(XmlUi::ToolStrip* strip);

public slots:
    void refresh();

public: // overrides
    bool eventFilter(QObject* watched, QEvent* e) override;

private slots:
    void driveChanged(const ShellDrive& drive);

private:
    struct StripInfo
    {
        XmlUi::ToolStrip* m_strip;
        QObject* m_receiver;
        std::function<void(int)> m_driveSelected;
        QList<QAction*> m_actions;
    };

private:
    void populateToolStrip(StripInfo& info);

    bool stripDragEnterEvent(XmlUi::ToolStrip* strip, QDragEnterEvent* e);
    bool stripDragMoveEvent(XmlUi::ToolStrip* strip, QDragMoveEvent* e);
    bool stripDragLeaveEvent(XmlUi::ToolStrip* strip, QDragLeaveEvent* e);
    bool stripDropEvent(XmlUi::ToolStrip* strip, QDropEvent* e);

    bool dragDropHelper(XmlUi::ToolStrip* strip, QDropEvent* e, bool doDrop);

private:
    ShellComputer* m_computer{};
    QList<ShellDrive> m_drives;

    QList<StripInfo> m_strips;

    ShellDropData* m_dropData{};

    friend class ShellDriveLessThan;
};
