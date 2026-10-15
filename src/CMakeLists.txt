#############################################################################
# Saladin executable
#############################################################################

# --- application -----------------------------------------------------------

set(SALADIN_SOURCES
    aboutbox.cpp
    application.cpp
    bookmark.cpp
    bookmarksdialog.cpp
    drivestripmanager.cpp
    findbar.cpp
    folderitemdelegate.cpp
    folderitemmodel.cpp
    folderitemview.cpp
    guidedialog.cpp
    main.cpp
    mainwindow.cpp
    multirenamewidget.cpp
    openftpdialog.cpp
    operationdialog.cpp
    panewidget.cpp
    searchdialog.cpp
    searchhelper.cpp
    searchitemmodel.cpp
    settingsdialog.cpp
    windark.cpp
)

set(SALADIN_HEADERS
    aboutbox.h
    application.h
    bookmark.h
    bookmark_p.h
    bookmarksdialog.h
    drivestripmanager.h
    findbar.h
    folderitemdelegate.h
    folderitemmodel.h
    folderitemview.h
    guidedialog.h
    mainwindow.h
    multirenamewidget.h
    openftpdialog.h
    operationdialog.h
    panewidget.h
    searchdialog.h
    searchhelper.h
    searchitemmodel.h
    settingsdialog.h
    windark.h
    precompiled.h
)

# --- shell/ ----------------------------------------------------------------

list(APPEND SALADIN_SOURCES
    shell/changenotifywatcher.cpp
    shell/iconcache.cpp
    shell/shellbase.cpp
    shell/shellcomputer.cpp
    shell/shelldrive.cpp
    shell/shelldropdata.cpp
    shell/shellfolder.cpp
    shell/shellitem.cpp
    shell/shellitem_p.cpp
    shell/shellpidl.cpp
    shell/shellselection.cpp
    shell/streamdevice.cpp
)

list(APPEND SALADIN_HEADERS
    shell/changenotifywatcher_p.h
    shell/iconcache_p.h
    shell/shellbase_p.h
    shell/shellcomputer.h
    shell/shellcomputer_p.h
    shell/shelldrive.h
    shell/shelldrive_p.h
    shell/shelldropdata.h
    shell/shelldropdata_p.h
    shell/shellfolder.h
    shell/shellfolder_p.h
    shell/shellitem.h
    shell/shellitem_p.h
    shell/shellpidl.h
    shell/shellpidl_p.h
    shell/shellselection.h
    shell/shellselection_p.h
    shell/streamdevice.h
    shell/streamdevice_p.h
)

# --- utils/ ----------------------------------------------------------------

list(APPEND SALADIN_SOURCES
    utils/dataserializer.cpp
    utils/elidedlabel.cpp
    utils/formathelper.cpp
    utils/iconloader.cpp
    utils/localsettings.cpp
    utils/separatorcombobox.cpp
    utils/textencoding.cpp
    utils/updateclient.cpp
)

list(APPEND SALADIN_HEADERS
    utils/dataserializer.h
    utils/elidedlabel.h
    utils/formathelper.h
    utils/iconloader.h
    utils/localsettings.h
    utils/separatorcombobox.h
    utils/textencoding.h
    utils/updateclient.h
)

# --- viewer/ ---------------------------------------------------------------

list(APPEND SALADIN_SOURCES
    viewer/binaryloader.cpp
    viewer/binaryview.cpp
    viewer/gotodialog.cpp
    viewer/imagelabel.cpp
    viewer/imageloader.cpp
    viewer/imageview.cpp
    viewer/textedit.cpp
    viewer/textloader.cpp
    viewer/textview.cpp
    viewer/view.cpp
    viewer/viewerwindow.cpp
    viewer/viewmanager.cpp
)

list(APPEND SALADIN_HEADERS
    viewer/binaryloader.h
    viewer/binaryview.h
    viewer/gotodialog.h
    viewer/imagelabel.h
    viewer/imageloader.h
    viewer/imageview.h
    viewer/textedit.h
    viewer/textloader.h
    viewer/textview.h
    viewer/view.h
    viewer/viewerwindow.h
    viewer/viewmanager.h
)

# --- xmlui/ ----------------------------------------------------------------

list(APPEND SALADIN_SOURCES
    xmlui/builder.cpp
    xmlui/client.cpp
    xmlui/gradientwidget.cpp
    xmlui/toolstrip.cpp
    xmlui/windowsstyle.cpp
)

list(APPEND SALADIN_HEADERS
    xmlui/builder.h
    xmlui/client.h
    xmlui/gradientwidget.h
    xmlui/node_p.h
    xmlui/toolstrip.h
    xmlui/toolstrip_p.h
    xmlui/windowsstyle.h
)

# --- custom_widgets/ -------------------------------------------------------

list(APPEND SALADIN_SOURCES
    custom_widgets/PathLineEditor.cpp
)

list(APPEND SALADIN_HEADERS
    custom_widgets/PathLineEditor.h
)

# --- resources -------------------------------------------------------------

set(SALADIN_RESOURCES
    saladin.rc
    guide/guide.qrc
    icons/icons.qrc
    icons/dark/dark.qrc
    resources/resources.qrc
)

# --- target ----------------------------------------------------------------

qt_add_executable(saladin WIN32
    ${SALADIN_SOURCES}
    ${SALADIN_HEADERS}
    ${SALADIN_RESOURCES}
)

target_include_directories(saladin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_precompile_headers(saladin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h")

target_link_libraries(saladin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
    Qt6::CorePrivate
    Qt6::GuiPrivate
    Qt6::WidgetsPrivate
    shell32
    shlwapi
    crypt32
    ole32
    oleaut32
    uuid
)

install(TARGETS saladin RUNTIME DESTINATION bin)
