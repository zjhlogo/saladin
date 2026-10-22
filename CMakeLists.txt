#############################################################################
# Saladin - dual-pane file manager for Windows
#
# CMake build definition (replaces the former qmake project files).
#
# Quick start (Qt 6 + MSVC):
#
#   rmdir /s /q build
#   cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
#   cmake --build build --config Release
#
# From a normal prompt -G Ninja is faster.
# (Ninja cannot be used inside the DSH workspace sandbox: it hangs there.)
#
# The executable is written to release/ (or debug/) inside the source tree, see
# "Output layout" below.
#############################################################################

cmake_minimum_required(VERSION 3.21)

project(Saladin
    VERSION 1.0.0
    DESCRIPTION "Dual-pane file manager for Windows"
    LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Saladin is a Windows-only application.")
endif()

enable_language(RC)

# ---------------------------------------------------------------------------
# Qt
# ---------------------------------------------------------------------------

find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core Gui Widgets Network Xml Svg
    CorePrivate GuiPrivate WidgetsPrivate)

# Enables AUTOMOC and AUTOUIC and selects a suitable C++ standard.
qt_standard_project_setup()

# qt_standard_project_setup() does not turn on AUTORCC, but the .qrc files below
# are compiled the same way qmake's RESOURCES were, so enable it explicitly.
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------------------------
# Default build type (single-config generators)
# ---------------------------------------------------------------------------

get_property(SALADIN_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(NOT SALADIN_IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# ---------------------------------------------------------------------------
# Output layout
#
# Mirrors the historical qmake layout (DESTDIR = ../release, ../debug) so that
# the deployed Qt runtime and the installer scripts keep working unchanged.
# ---------------------------------------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_SOURCE_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_SOURCE_DIR}/release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_SOURCE_DIR}/release")

# ---------------------------------------------------------------------------
# Qt runtime deployment
#
# After every successful link the Qt runtime files (DLLs, the runtime plugins
# used by Saladin and the runtime language files from translations/) are copied
# next to the executable, so debug/ and release/ contain a directly runnable
# program without a Qt environment, using the deployed language files.
#
# The deployed file set is defined below and implemented in
# cmake/DeployQtRuntime.cmake; it mirrors the runtime installed by
# installer/saladin.nsi, updated for Qt 6. Disable it (e.g. for builds that
# collect the runtime separately) with:
#
#   cmake -S . -B build -DSALADIN_DEPLOY_QT_RUNTIME=OFF
#
# Note that the Visual C++ runtime DLLs are not copied; install the Visual
# C++ redistributable or use the installer for deployment to clean machines.
# ---------------------------------------------------------------------------

option(SALADIN_DEPLOY_QT_RUNTIME "Copy Qt runtime files next to the executable after building" ON)

set(SALADIN_QT_MODULES
    Core Gui Widgets Network Xml Svg)

# Runtime plugins, given as paths relative to the Qt plugins directory and
# without extension. The deploy script copies each <path>[d].dll that exists
# in the Qt installation (plugins absent from the package are skipped).
set(SALADIN_QT_PLUGINS
    platforms/qwindows
    styles/qmodernwindowsstyle
    styles/qwindowsvistastyle
    imageformats/qgif
    imageformats/qico
    imageformats/qjpeg
    imageformats/qsvg
    imageformats/qtiff
    imageformats/qwebp
    tls/qschannelbackend
    networkinformation/qnetworklistmanager)

# MinGW toolchain DLLs shipped in the Qt binary directory (ignored on MSVC).
set(SALADIN_QT_MINGW_DLLS
    libgcc_s_seh-1.dll
    libstdc++-6.dll
    libwinpthread-1.dll)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

add_subdirectory(src)

# ---------------------------------------------------------------------------
# Plugins
#
# User-visible extension DLLs, loaded at runtime from "<exe dir>/plugins"
# (system-wide) or "<AppData>/Saladin/plugins" (per user). Every plugin is
# deployed into its own folder below plugins/ - "<plugins>/<target>/" holds
# the DLL together with its resources (UI XML, translations). See
# plugins/README.md.
# ---------------------------------------------------------------------------

option(SALADIN_BUILD_EXAMPLE_PLUGIN "Build the example Saladin plugin" ON)

if(SALADIN_BUILD_EXAMPLE_PLUGIN)
    add_subdirectory(plugins/example)
endif()

option(SALADIN_BUILD_FTP_PLUGIN "Build the FTP Saladin plugin (Connect To FTP)" ON)

if(SALADIN_BUILD_FTP_PLUGIN)
    add_subdirectory(plugins/ftp)
endif()

# ---------------------------------------------------------------------------
# Translations
#
# The translation catalogs are maintained outside CMake with two helper
# scripts, which replace the former "saladin_lupdate", "saladin_lrelease" and
# "saladin_sync_translations" build targets (plus their global aliases
# "update_translations" and "release_translations"):
#
#   python tools/update_translations.py    # recollect the strings from the
#                                          # sources into the .ts files
#   python tools/release_translations.py   # recompile .ts -> .qm
#
# Both scripts locate the Qt Linguist tools on their own (see tools/). The .qm
# files are written directly into the source tree - the same translations/
# directory the Qt runtime deployment copies next to the executable - so no
# separate sync step is needed, and a normal build depends neither on Python
# nor on the Linguist tools.
# ---------------------------------------------------------------------------

set(SALADIN_LANGUAGES zh_CN)

# The .qm files are committed to the repository and only rebuilt on demand
# with the script above; complain about missing ones at configure time.
set(SALADIN_QM_FILES "")
foreach(lang IN LISTS SALADIN_LANGUAGES)
    set(qm "${CMAKE_CURRENT_SOURCE_DIR}/translations/saladin_${lang}.qm")
    if(EXISTS "${qm}")
        list(APPEND SALADIN_QM_FILES "${qm}")
    else()
        message(WARNING "Missing translation: ${qm} - run 'python tools/release_translations.py' to generate it")
    endif()
endforeach()

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/locale.ini"
    ${SALADIN_QM_FILES}
    DESTINATION translations)
