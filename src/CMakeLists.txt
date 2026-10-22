#############################################################################
# Saladin executable
#############################################################################

# --- application -----------------------------------------------------------

set(SALADIN_SOURCES
        about_box.cpp
        application.cpp
        bookmark.cpp
        bookmarks_dialog.cpp
        drive_strip_manager.cpp
        find_bar.cpp
        folder_item_delegate.cpp
        folder_item_model.cpp
        folder_item_view.cpp
        guide_dialog.cpp
        main.cpp
        main_window.cpp
        multi_rename_widget.cpp
        open_ftp_dialog.cpp
        operation_dialog.cpp
        panel_widget.cpp
        search_dialog.cpp
        search_helper.cpp
        search_item_model.cpp
        settings_dialog.cpp
)

set(SALADIN_HEADERS
        about_box.h
        application.h
        bookmark.h
        bookmark_p.h
        bookmarks_dialog.h
        drive_strip_manager.h
        find_bar.h
        folder_item_delegate.h
        folder_item_model.h
        folder_item_view.h
        guide_dialog.h
        main_window.h
        multi_rename_widget.h
        open_ftp_dialog.h
        operation_dialog.h
        panel_widget.h
        search_dialog.h
        search_helper.h
        search_item_model.h
        settings_dialog.h
        precompiled.h
)

# --- shell/ ----------------------------------------------------------------

list(APPEND SALADIN_SOURCES
        shell/change_notify_watcher.cpp
        shell/icon_cache.cpp
        shell/shell_base.cpp
        shell/shell_computer.cpp
        shell/shell_drive.cpp
        shell/shell_drop_data.cpp
        shell/shell_folder.cpp
        shell/shell_item.cpp
        shell/shell_item_p.cpp
        shell/shell_pidl.cpp
        shell/shell_selection.cpp
        shell/stream_device.cpp
)

list(APPEND SALADIN_HEADERS
        shell/change_notify_watcher_p.h
        shell/icon_cache_p.h
        shell/shell_base_p.h
        shell/shell_computer.h
        shell/shell_computer_p.h
        shell/shell_drive.h
        shell/shell_drive_p.h
        shell/shell_drop_data.h
        shell/shell_drop_data_p.h
        shell/shell_folder.h
        shell/shell_folder_p.h
        shell/shell_item.h
        shell/shell_item_p.h
        shell/shell_pidl.h
        shell/shell_pidl_p.h
        shell/shell_selection.h
        shell/shell_selection_p.h
        shell/stream_device.h
        shell/stream_device_p.h
)

# --- utils/ ----------------------------------------------------------------

list(APPEND SALADIN_SOURCES
        utils/data_serializer.cpp
        utils/elided_label.cpp
        utils/format_helper.cpp
        utils/icon_loader.cpp
        utils/local_settings.cpp
        utils/path_line_editor.cpp
        utils/separator_combo_box.cpp
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
        utils/separator_combo_box.h
        utils/text_encoding.h
        utils/theme_helper.h
        utils/update_client.h
)

# --- custom_widgets/ -------------------------------------------------------

list(APPEND SALADIN_SOURCES
        custom_widgets/side_tab_bar.cpp
)

list(APPEND SALADIN_HEADERS
        custom_widgets/side_tab_bar.h
)

# --- viewer/ ---------------------------------------------------------------

list(APPEND SALADIN_SOURCES
        viewer/binary_loader.cpp
        viewer/binary_view.cpp
        viewer/goto_dialog.cpp
        viewer/image_label.cpp
        viewer/image_loader.cpp
        viewer/image_view.cpp
        viewer/text_edit.cpp
        viewer/text_loader.cpp
        viewer/text_view.cpp
        viewer/view.cpp
        viewer/viewer_window.cpp
        viewer/view_manager.cpp
)

list(APPEND SALADIN_HEADERS
        viewer/binary_loader.h
        viewer/binary_view.h
        viewer/goto_dialog.h
        viewer/image_label.h
        viewer/image_loader.h
        viewer/image_view.h
        viewer/text_edit.h
        viewer/text_loader.h
        viewer/text_view.h
        viewer/view.h
        viewer/viewer_window.h
        viewer/view_manager.h
)

# --- xmlui/ ----------------------------------------------------------------

list(APPEND SALADIN_SOURCES
        xmlui/builder.cpp
        xmlui/client.cpp
        xmlui/gradient_widget.cpp
        xmlui/tool_strip.cpp
        xmlui/windows_style.cpp
)

list(APPEND SALADIN_HEADERS
        xmlui/builder.h
        xmlui/client.h
        xmlui/gradient_widget.h
        xmlui/node_p.h
        xmlui/tool_strip.h
        xmlui/tool_strip_p.h
        xmlui/windows_style.h
)

# --- plugin/ ---------------------------------------------------------------

list(APPEND SALADIN_SOURCES
        plugin/plugin_host.cpp
        plugin/plugin_manager.cpp
)

list(APPEND SALADIN_HEADERS
        plugin/plugin_api.h
        plugin/plugin_host.h
        plugin/plugin_info.h
        plugin/plugin_manager.h
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
