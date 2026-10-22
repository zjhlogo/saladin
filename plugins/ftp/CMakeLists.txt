#############################################################################
# FTP Saladin plugin - "Connect To FTP..." action and connection dialog
#
# The FTP connection feature moved out of the host (formerly the built-in
# openFtpSite action plus src/open_ftp_dialog.*). The plugin links only Qt
# and includes the header-only API from src/plugin/; it never links against
# the saladin executable itself.
#############################################################################

# The host scans "<executable dir>/plugins\<plugin folder>", which mirrors the
# in-source-tree output layout of the saladin target (release/ or debug/):
# each plugin lives in its own subdirectory below plugins/, named after the
# plugin target (its DLL), so the DLL and its resources travel together.
qt_add_plugin(ftp_plugin SHARED
        ftp_plugin.h
        ftp_plugin.cpp
        ftp_dialog.h
        ftp_dialog.cpp
        icons/icons.qrc
        metadata.json
        plugin.xml
)

target_include_directories(ftp_plugin PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(ftp_plugin PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
        Qt6::Widgets
)

# MODULE libraries place their DLL by LIBRARY_OUTPUT_DIRECTORY (the import
# .lib goes to ARCHIVE, so all three are set, exactly like Qt's own plugin
# helpers do). Debug goes to debug/plugins/ftp_plugin, everything else to
# release/plugins/ftp_plugin.
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    if(_cfg STREQUAL "DEBUG")
        set(_dir "${CMAKE_SOURCE_DIR}/debug/plugins/ftp_plugin")
    else()
        set(_dir "${CMAKE_SOURCE_DIR}/release/plugins/ftp_plugin")
    endif()
    set_target_properties(ftp_plugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "ARCHIVE_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "RUNTIME_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
    )
endforeach()

# The UI layout referenced by "ui" in metadata.json is read at runtime from the
# DLL's own directory (the host resolves it relative to the plugin path), so it
# is copied into the plugin folder after the link and travels with the plugin
# wherever it is deployed.
add_custom_command(TARGET ftp_plugin POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml"
                "$<TARGET_FILE_DIR:ftp_plugin>"
)

# Bundled catalogs (plugins/ftp/translations/ftp_*.ts -> ftp_*.qm) are
# maintained outside CMake with the shared helper scripts, like the host
# catalogs: tools/update_translations.py recollects the strings (creating the
# .ts files for the application's languages), tools/release_translations.py
# compiles the .qm files. The plugin opts in via "translations": true in
# metadata.json. The committed .qm files are deployed like the application's
# runtime catalogs: copied after the link into "<plugin dir>/translations"
# next to the DLL - the directory FtpPlugin::loadTranslation (ftp_plugin.cpp)
# loads them from at runtime.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/translations")
    file(GLOB _ftp_qm_files "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.qm")

    if(_ftp_qm_files)
        set(_ftp_deploy_commands
                COMMAND "${CMAKE_COMMAND}" -E make_directory
                        "$<TARGET_FILE_DIR:ftp_plugin>/translations")

        foreach(_qm IN LISTS _ftp_qm_files)
            list(APPEND _ftp_deploy_commands
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                            "${_qm}"
                            "$<TARGET_FILE_DIR:ftp_plugin>/translations")
        endforeach()

        add_custom_command(TARGET ftp_plugin POST_BUILD ${_ftp_deploy_commands})
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/translations/"
            DESTINATION bin/plugins/ftp_plugin/translations
            FILES_MATCHING PATTERN "*.qm")
endif()

install(TARGETS ftp_plugin
        LIBRARY DESTINATION bin/plugins/ftp_plugin
        RUNTIME DESTINATION bin/plugins/ftp_plugin)

install(FILES plugin.xml DESTINATION bin/plugins/ftp_plugin)
