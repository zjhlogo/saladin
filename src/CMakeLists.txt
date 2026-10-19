#############################################################################
# Saladin executable
#############################################################################

# --- application -----------------------------------------------------------

set(SALADIN_SOURCES
        about_box.cpp
        application.cpp
        bookmark.cpp
        bookmarks_dialog.cpp
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
        settings_dialog.cpp
)

set(SALADIN_HEADERS
        about_box.h
        application.h
        bookmark.h
        bookmark_p.h
        bookmarks_dialog.h
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
        settings_dialog.h
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
        utils/data_serializer.cpp
        utils/elided_label.cpp
        utils/format_helper.cpp
        utils/icon_loader.cpp
        utils/local_settings.cpp
        utils/path_line_editor.cpp
        utils/separator_combobox.cpp
        utils/text_encoding.cpp
        utils/theme_helper.cpp
        utils/update_client.cpp
)

list(APPEND SALADIN_HEADERS
        utils/data_serializer.h
        utils/elided_label.h
        utils/format_helper.h
        utils/icon_loader.h
        utils/local_settings.h
        utils/path_line_editor.h
        utils/separator_combobox.h
        utils/text_encoding.h
        utils/theme_helper.h
        utils/update_client.h
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

# --- resources -------------------------------------------------------------

set(SALADIN_RESOURCES
        saladin.rc
        guide/guide.qrc
        icons/icons.qrc
        icons/dark/dark.qrc
        icons/material/material.qrc
        resources/resources.qrc
        themes/qt-material/themes.qrc
)

# --- target ----------------------------------------------------------------

qt_add_executable(saladin WIN32
        ${SALADIN_SOURCES}
        ${SALADIN_HEADERS}
        ${SALADIN_RESOURCES}
)

target_include_directories(saladin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Keep the translation catalogs current when the application is built directly
# (they are the source of the copy deployed next to the executable, and the
# fallback the application uses when the deployed copy is absent).
if (TARGET saladin_sync_translations)
    add_dependencies(saladin saladin_sync_translations)
endif ()

target_precompile_headers(saladin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h")

target_link_libraries(saladin PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Xml
        Qt6::Svg
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

# --- Qt runtime deployment -------------------------------------------------
#
# Copy the Qt runtime next to the executable after every successful link, so
# debug/saladin.exe and release/saladin.exe can be started without a Qt
# environment. The translation catalogs (saladin_*.qm, locale.ini) are part of
# that runtime and are deployed to "<executable dir>/translations", see
# "Qt runtime deployment" in the top-level CMakeLists.txt.

if (SALADIN_DEPLOY_QT_RUNTIME)
    # Qt binary and plugins directories, taken from the Qt6::Core target.
    get_target_property(_qt_bin_dir Qt6::Core QT6_INSTALL_BINS)
    get_target_property(_qt_plugin_dir Qt6::Core QT6_INSTALL_PLUGINS)
    if (NOT _qt_bin_dir)
        # Fallback: derive the binary directory from the qmake location.
        get_target_property(_qt_qmake Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin_dir "${_qt_qmake}" DIRECTORY)
    endif ()
    if (NOT _qt_plugin_dir)
        get_filename_component(_qt_plugin_dir "${_qt_bin_dir}/../plugins" ABSOLUTE)
    endif ()

    add_custom_command(TARGET saladin POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            "-DQT_RUNTIME_BIN_DIR=${_qt_bin_dir}"
            "-DQT_RUNTIME_PLUGIN_DIR=${_qt_plugin_dir}"
            "-DQT_RUNTIME_OUTPUT_DIR=$<TARGET_FILE_DIR:saladin>"
            "-DQT_RUNTIME_MODULES=${SALADIN_QT_MODULES}"
            "-DQT_RUNTIME_PLUGINS=${SALADIN_QT_PLUGINS}"
            "-DQT_RUNTIME_MINGW_DLLS=${SALADIN_QT_MINGW_DLLS}"
            "-DQT_RUNTIME_DEBUG=$<CONFIG:Debug>"
            "-DQT_RUNTIME_MSVC=$<BOOL:${MSVC}>"
            "-DQT_RUNTIME_TRANSLATIONS_DIR=${CMAKE_SOURCE_DIR}/translations"
            -P "${CMAKE_SOURCE_DIR}/cmake/DeployQtRuntime.cmake"
            COMMENT "Deploying Qt runtime files"
            VERBATIM
    )
endif ()
