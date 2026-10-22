#############################################################################
# Example Saladin plugin (also the template for new plugins)
#
# The plugin links only Qt and includes the header-only API from src/plugin/.
# It never links against the saladin executable itself.
#############################################################################

# The host scans "<executable dir>/plugins\<plugin folder>", which mirrors the
# in-source-tree output layout of the saladin target (release/ or debug/):
# each plugin lives in its own subdirectory below plugins/, named after the
# plugin target (its DLL), so the DLL and its resources travel together.
qt_add_plugin(example_plugin SHARED
        example_plugin.h
        example_plugin.cpp
        metadata.json
        plugin.xml
)

target_include_directories(example_plugin PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(example_plugin PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# MODULE libraries place their DLL by LIBRARY_OUTPUT_DIRECTORY (the import .lib
# goes to ARCHIVE, so all three are set, exactly like Qt's own plugin helpers
# do). Debug goes to debug/plugins/example_plugin, everything else to
# release/plugins/example_plugin.
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    if(_cfg STREQUAL "DEBUG")
        set(_dir "${CMAKE_SOURCE_DIR}/debug/plugins/example_plugin")
    else()
        set(_dir "${CMAKE_SOURCE_DIR}/release/plugins/example_plugin")
    endif()
    set_target_properties(example_plugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "ARCHIVE_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "RUNTIME_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
    )
endforeach()

# The UI layout referenced by "ui" in metadata.json is read at runtime from the
# DLL's own directory (the host resolves it relative to the plugin path), so it
# is copied into the plugin folder after the link and travels with the plugin
# wherever it is deployed.
add_custom_command(TARGET example_plugin POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml"
                "$<TARGET_FILE_DIR:example_plugin>"
)

install(TARGETS example_plugin
        LIBRARY DESTINATION bin/plugins/example_plugin
        RUNTIME DESTINATION bin/plugins/example_plugin)

install(FILES plugin.xml DESTINATION bin/plugins/example_plugin)
