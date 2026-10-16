#############################################################################
# Qt runtime deployment helper
#
# Invoked as a POST_BUILD step of the saladin target (see src/CMakeLists.txt
# and "Qt runtime deployment" in the top-level CMakeLists.txt). Copies the
# Qt 6 runtime DLLs and the runtime plugins used by Saladin next to the
# executable, so it can be started directly from debug/ or release/ without
# a Qt environment.
#
# The file set mirrors the runtime installed by installer/saladin.nsi,
# updated for Qt 6 (Qt5WinExtras is gone, the SSL backend is now the
# schannel plugin shipped with Qt, and image formats are chosen by the
# image viewer).
#
# windeployqt is intentionally not used: it does not support multi-config
# output directories well and requires spawning helper processes.
#
# Inputs, all passed with -D:
#   QT_RUNTIME_BIN_DIR    - Qt binary directory (Qt6Core.dll, ...)
#   QT_RUNTIME_PLUGIN_DIR - Qt plugins directory (platforms/, styles/, ...)
#   QT_RUNTIME_OUTPUT_DIR - target directory (where saladin.exe was linked)
#   QT_RUNTIME_MODULES    - Qt modules to copy: Core;Gui;...
#   QT_RUNTIME_PLUGINS    - plugin files relative to QT_RUNTIME_PLUGIN_DIR,
#                           without extension: platforms/qwindows;...
#   QT_RUNTIME_MINGW_DLLS - MinGW toolchain DLLs, copied when present
#   QT_RUNTIME_DEBUG      - 1 for debug builds, 0 otherwise
#   QT_RUNTIME_MSVC       - 1 when building with MSVC, 0 otherwise
#############################################################################

cmake_minimum_required(VERSION 3.21)

if(NOT QT_RUNTIME_BIN_DIR OR NOT QT_RUNTIME_OUTPUT_DIR)
    message(FATAL_ERROR "DeployQtRuntime: QT_RUNTIME_BIN_DIR and QT_RUNTIME_OUTPUT_DIR are required.")
endif()

# MSVC appends "d" to debug DLLs and plugins. MinGW builds (and the official
# MinGW packages) use unsuffixed names in all configurations.
set(_suffix "")
if(QT_RUNTIME_DEBUG AND QT_RUNTIME_MSVC)
    set(_suffix "d")
endif()

# --- Qt module DLLs --------------------------------------------------------

foreach(_module IN LISTS QT_RUNTIME_MODULES)
    set(_src "${QT_RUNTIME_BIN_DIR}/Qt6${_module}${_suffix}.dll")
    set(_dst "${QT_RUNTIME_OUTPUT_DIR}/Qt6${_module}${_suffix}.dll")
    if(EXISTS "${_src}")
        file(COPY_FILE "${_src}" "${_dst}" ONLY_IF_DIFFERENT)
    else()
        message(WARNING "DeployQtRuntime: Qt runtime DLL not found: ${_src}")
    endif()
endforeach()

# MinGW toolchain DLLs shipped in the Qt binary directory (no-op on MSVC).
foreach(_dll IN LISTS QT_RUNTIME_MINGW_DLLS)
    if(EXISTS "${QT_RUNTIME_BIN_DIR}/${_dll}")
        file(COPY_FILE "${QT_RUNTIME_BIN_DIR}/${_dll}" "${QT_RUNTIME_OUTPUT_DIR}/${_dll}" ONLY_IF_DIFFERENT)
    endif()
endforeach()

# --- runtime plugins -------------------------------------------------------
#
# Each entry is a path below the Qt plugins directory without extension, for
# example "platforms/qwindows". The matching <path>[d].dll is copied to the
# same relative location next to the executable. Plugins that are not part
# of the Qt package in use (optional image formats, TLS backends) are
# skipped silently.

foreach(_plugin IN LISTS QT_RUNTIME_PLUGINS)
    set(_src "${QT_RUNTIME_PLUGIN_DIR}/${_plugin}${_suffix}.dll")
    set(_dst "${QT_RUNTIME_OUTPUT_DIR}/${_plugin}${_suffix}.dll")
    if(EXISTS "${_src}")
        cmake_path(GET _plugin PARENT_PATH _group)
        if(_group)
            file(MAKE_DIRECTORY "${QT_RUNTIME_OUTPUT_DIR}/${_group}")
        endif()
        file(COPY_FILE "${_src}" "${_dst}" ONLY_IF_DIFFERENT)
    endif()
endforeach()
