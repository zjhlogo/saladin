#############################################################################
# Example Saladin plugin (also the template for new plugins)
#
# The plugin links only Qt and includes the header-only API from src/plugin/.
# It never links against the saladin executable itself.
#############################################################################

# The host scans "<executable dir>/plugins", which mirrors the in-source-tree
# output layout of the saladin target (release/ or debug/).
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

# The host scans "<executable dir>/plugins", which mirrors the in-source-tree
# output layout of the saladin target (release/ or debug/). MODULE libraries
# place their DLL by LIBRARY_OUTPUT_DIRECTORY (the import .lib goes to ARCHIVE,
# so all three are set, exactly like Qt's own plugin helpers do).
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    if(_cfg STREQUAL "DEBUG")
        set(_dir "${CMAKE_SOURCE_DIR}/debug/plugins")
    else()
        set(_dir "${CMAKE_SOURCE_DIR}/release/plugins")
    endif()
    set_target_properties(example_plugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "ARCHIVE_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
            "RUNTIME_OUTPUT_DIRECTORY_${_cfg}" "${_dir}"
    )
endforeach()

install(TARGETS example_plugin LIBRARY DESTINATION bin/plugins RUNTIME DESTINATION bin/plugins)
