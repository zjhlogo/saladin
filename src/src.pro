include( ../config.pri )

TEMPLATE = app
TARGET   = saladin

CONFIG  += qt
QT       = core-private gui-private widgets-private network xml winextras

HEADERS += aboutbox.h \
           application.h \
           bookmark.h \
           bookmark_p.h \
           bookmarksdialog.h \
           drivestripmanager.h \
           findbar.h \
           folderitemdelegate.h \
           folderitemmodel.h \
           folderitemview.h \
           guidedialog.h \
           mainwindow.h \
           multirenamewidget.h \
           openftpdialog.h \
           operationdialog.h \
           panewidget.h \
           searchdialog.h \
           searchhelper.h \
           searchitemmodel.h \
           settingsdialog.h \
           windark.h

SOURCES += aboutbox.cpp \
           application.cpp \
           bookmark.cpp \
           bookmarksdialog.cpp \
           drivestripmanager.cpp \
           findbar.cpp \
           folderitemdelegate.cpp \
           folderitemmodel.cpp \
           folderitemview.cpp \
           guidedialog.cpp \
           main.cpp \
           mainwindow.cpp \
           multirenamewidget.cpp \
           openftpdialog.cpp \
           operationdialog.cpp \
           panewidget.cpp \
           searchdialog.cpp \
           searchhelper.cpp \
           searchitemmodel.cpp \
           settingsdialog.cpp \
           windark.cpp

RESOURCES += \
           guide/guide.qrc \
           icons/icons.qrc \
           icons/dark/dark.qrc \
           resources/resources.qrc

include( shell/shell.pri )
include( utils/utils.pri )
include( viewer/viewer.pri )
include( xmlui/xmlui.pri )
include( custom_widgets/custom_widgets.pri )

INCLUDEPATH += .

PRECOMPILED_HEADER = precompiled.h

TRANSLATIONS += \
           ../translations/saladin_untranslated.ts \
           ../translations/saladin_de.ts \
           ../translations/saladin_es.ts \
           ../translations/saladin_pl.ts \
           ../translations/saladin_pt_BR.ts

RC_FILE = saladin.rc
LIBS += -lshell32 -lshlwapi -lcrypt32 -lole32 -loleaut32

win32-msvc* {
    QMAKE_CXXFLAGS += -Fd\$(IntDir)
    CONFIG -= flat
}

build_pass {
    MOC_DIR = ../tmp
    RCC_DIR = ../tmp
    UI_DIR = ../tmp
    CONFIG( debug, debug|release ) {
        OBJECTS_DIR = ../tmp/debug
        DESTDIR = ../debug
    } else {
        OBJECTS_DIR = ../tmp/release
        DESTDIR = ../release
    }
}

target.path = $${DESTINATION}$$PREFIX/bin
INSTALLS += target
