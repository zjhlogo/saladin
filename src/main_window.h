// \file main_window.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "shell/shell_selection.h"
#include "xmlui/client.h"

#include <QMainWindow>

class PanelWidget;
class DriveStripManager;
class ViewManager;
class ShellPidl;

class MainWindow : public QMainWindow, public XmlUi::Client
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() override;

public:
    enum Tool
    {
        ViewerTool,
        EditorTool,
        ConsoleTool,
        DiffTool
    };

public:
    DriveStripManager* driveStripManager() const { return m_driveStripManager; }

    ViewManager* viewManager() const { return m_viewManager; }

    PanelWidget* sourcePane() const { return m_sourcePane; }

    PanelWidget* pane(int index) const { return m_panes[index]; }

    void transferSelection(ShellSelection* selection, ShellFolder* targetFolder, ShellSelection::TransferType type, bool canRename);

    void startTool(Tool tool, ShellFolder* folder, const ShellItem& item);
    void startTool(Tool tool, const QString& path, const QString& directory);

    void gotoFile(const ShellPidl& folderPidl, const ShellItem& item);

public: // overrides
    bool eventFilter(QObject* object, QEvent* e) override;

protected: // overrides
    void closeEvent(QCloseEvent* e) override;

    void changeEvent(QEvent* e) override;

private slots:
    void configure();

    void paste();
    void cut();
    void copy();
    void copyNames();
    void copyPaths();
    void copyPathsRecursive();

    void closeTab();
    void duplicateTab();

    void refresh();
    void refreshDrives();
    void viewHidden(bool on);

    void applicationStateChanged(Qt::ApplicationState state);

    void openDirectory();
    void openParent();
    void openRoot();
    void openHome();

    void viewSameDirectory();
    void swapPanes();
    void copyToRightPane();
    void copyToLeftPane();

    void selectMask();
    void selectAll();
    void unselectMask();
    void unselectAll();
    void invertSelection();

    void renameCurrent();
    void viewCurrent();
    void viewSelected();
    void editCurrent();
    void editNew();
    void copySelected();
    void cloneSelected();
    void moveSelected();
    void renameSelected();
    void createFolder();
    void moveToTrashCan();
    void deleteSelected();
    void openTerminal();

    void packToZip();
    void browse();
    void calculateSize();
    void compareFiles();
    void compareDirectories();
    void search();
    void explore();

    void showHistory();
    void showBookmarks();
    void addBookmark();
    void editBookmarks();

    void showDrivesMenu1();
    void showDrivesMenu2();
    void showProperties();

    void openInLeftPanel();
    void openInRightPanel();
    void otherOpenFolder();
    void otherOpenParent();

    void hideToolStrip();
    void showToolStrip();

    void loadIcons();

private:
    void initializeSettings();
    void saveSettings();

    void setSourcePane(int index);

    void invokeCommand(ShellFolder* folder, const QList<ShellItem>& items, const char* verb);

    void transferItems(ShellFolder* sourceFolder, const QList<ShellItem>& items, ShellFolder* targetFolder, ShellSelection::TransferType type);
    void deleteItems(ShellFolder* folder, const QList<ShellItem>& items, ShellSelection::Flags flags);

    QString toolPath(Tool tool);
    QString toolName(Tool tool);

    void createToolStrip();
    void createMenuBar();

private:
    DriveStripManager* m_driveStripManager{};

    ViewManager* m_viewManager{};

    PanelWidget* m_panes[2]{};

    PanelWidget* m_sourcePane{};
    PanelWidget* m_targetPane{};
};

extern MainWindow* g_mainWindow;
