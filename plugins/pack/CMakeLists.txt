#############################################################################
# Pack Saladin plugin - "Pack..." action and ZIP creation dialog
#
# The ZIP packing feature (formerly the built-in packToZip action plus its
# OperationDialog usage in src/main_window.cpp) moved out of the host. The
# plugin links only Qt and the Windows shell libraries; it never links against
# the saladin executable itself.
#############################################################################

# The host scans "<executable dir>/plugins\<plugin folder>", which mirrors the
# in-source-tree output layout of the saladin target (release/ or debug/):
# each plugin lives in its own subdirectory below plugins/, named after the
# plugin target (its DLL), so the DLL and its resources travel together.
qt_add_plugin(pack_plugin SHARED
        pack_plugin.h
        pack_plugin.cpp
        pack_dialog.h
        pack_dialog.cpp
        zip_packer.h
        zip_packer.cpp
        icons/icons.qrc
        metadata.json
        plugin.xml
)

target_include_directories(pack_plugin PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(pack_plugin PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
        Qt6::Widgets
        shell32
        ole32
        uuid
)

# MODULE libraries place their DLL by LIBRARY_OUTPUT_DIRECTORY (the import
# .lib goes to ARCHIVE, so all three are set, exactly like Qt's own plugin
# helpers do). Debug goes to debug/plugins/pack_plugin, everything else to
# release/plugins/pack_plugin.
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    if(_cfg STREQUAL "DEBUG")
        set(_dir "${CMAKE_SOURCE_DIR}/debug/plugins/pack_plugin")
    else()
        set(_dir "${CMAKE_SOURCE_DIR}/release/plugins/pack_plugin")
    endif()
    set_target_properties(pack_plugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "ARCHIVE_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "RUNTIME_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
    )
endforeach()

# The UI layout referenced by "ui" in metadata.json is read at runtime from the
# DLL's own directory (the host resolves it relative to the plugin path), so it
# is copied into the plugin folder after the link and travels with the plugin
# wherever it is deployed.
add_custom_command(TARGET pack_plugin POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml"
                "$<TARGET_FILE_DIR:pack_plugin>"
)

# Bundled catalogs (plugins/pack/translations/pack_*.ts -> pack_*.qm) are
# maintained outside CMake with the shared helper scripts, like the host
# catalogs: tools/update_translations.py recollects the strings,
# tools/release_translations.py compiles the .qm files. The plugin opts in via
# "translations": true in metadata.json. The .qm files are deployed after the
# link into "<plugin dir>/translations" next to the DLL - the directory
# PackPlugin::loadTranslation (pack_plugin.cpp) loads them from at runtime.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/translations")
    file(GLOB _pack_qm_files "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.qm")

    if(_pack_qm_files)
        set(_pack_deploy_commands
                COMMAND "${CMAKE_COMMAND}" -E make_directory
                        "$<TARGET_FILE_DIR:pack_plugin>/translations")

        foreach(_qm IN LISTS _pack_qm_files)
            list(APPEND _pack_deploy_commands
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                            "${_qm}"
                            "$<TARGET_FILE_DIR:pack_plugin>/translations")
        endforeach()

        add_custom_command(TARGET pack_plugin POST_BUILD ${_pack_deploy_commands})
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/translations/"
            DESTINATION bin/plugins/pack_plugin/translations
            FILES_MATCHING PATTERN "*.qm")
endif()

install(TARGETS pack_plugin
        LIBRARY DESTINATION bin/plugins/pack_plugin
        RUNTIME DESTINATION bin/plugins/pack_plugin)

install(FILES plugin.xml DESTINATION bin/plugins/pack_plugin)
