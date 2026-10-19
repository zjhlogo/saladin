#############################################################################
# Qt runtime deployment helper
#
# Invoked as a POST_BUILD step of the saladin target (see src/CMakeLists.txt
# and "Qt runtime deployment" in the top-level CMakeLists.txt). Copies the
# Qt 6 runtime DLLs, the runtime plugins used by Saladin and the translation
# catalogs next to the executable, so it can be started directly from debug/
# or release/ without a Qt environment and with the deployed language files.
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
#   QT_RUNTIME_TRANSLATIONS_DIR - source translations/ directory whose runtime
#                           files (saladin_*.qm, locale.ini) are copied into
#                           <output dir>/translations
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

# --- runtime translations --------------------------------------------------
#
# The translation catalogs (saladin_*.qm) and the language list (locale.ini)
# are runtime files just like the DLLs: they are deployed as
# "<executable dir>/translations", which is the directory the application
# reads first (Application::initializeDefaultPaths). Without this copy the
# executable in debug/ or release/ would fall back to the catalogs of the
# source tree one level up, and the deployed language files would not be the
# ones in use.

if(NOT QT_RUNTIME_TRANSLATIONS_DIR)
    message(WARNING "DeployQtRuntime: QT_RUNTIME_TRANSLATIONS_DIR not set, translations were not deployed.")
elseif(NOT IS_DIRECTORY "${QT_RUNTIME_TRANSLATIONS_DIR}")
    message(WARNING "DeployQtRuntime: translation directory not found: ${QT_RUNTIME_TRANSLATIONS_DIR}")
else()
    set(_translations_dst "${QT_RUNTIME_OUTPUT_DIR}/translations")
    file(MAKE_DIRECTORY "${_translations_dst}")

    file(GLOB _translation_files
        "${QT_RUNTIME_TRANSLATIONS_DIR}/saladin_*.qm"
        "${QT_RUNTIME_TRANSLATIONS_DIR}/locale.ini")

    foreach(_file IN LISTS _translation_files)
        get_filename_component(_name "${_file}" NAME)
        file(COPY_FILE "${_file}" "${_translations_dst}/${_name}" ONLY_IF_DIFFERENT)
    endforeach()

    list(LENGTH _translation_files _translation_count)
    message(STATUS "DeployQtRuntime: deployed ${_translation_count} translation files to ${_translations_dst}")
endif()
