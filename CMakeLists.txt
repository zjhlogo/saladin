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
# (system-wide) or "<AppData>/Saladin/plugins" (per user). See plugins/README.md.
# ---------------------------------------------------------------------------

option(SALADIN_BUILD_EXAMPLE_PLUGIN "Build the example Saladin plugin" ON)

if(SALADIN_BUILD_EXAMPLE_PLUGIN)
    add_subdirectory(plugins/example)
endif()

# ---------------------------------------------------------------------------
# Translations
#
# Must come after add_subdirectory(src): the lupdate target reads the source
# list of the "saladin" target.
#
#   cmake --build build --target saladin_lupdate       # recollect the strings
#                                                      # from the sources into
#                                                      # the .ts files
#   cmake --build build --target saladin_lrelease      # recompile .ts -> .qm
#
# Both targets also exist under their global aliases "update_translations"
# and "release_translations" (added by Qt's LinguistTools macros). The lupdate
# run passes -no-obsolete so strings removed from the sources are dropped from
# the catalogs; remove the option to keep them as "vanished" entries instead.
# ---------------------------------------------------------------------------

set(SALADIN_LANGUAGES de es hu pl pt_BR ru zh_CN)

set(SALADIN_TS_FILES "")
foreach(lang IN LISTS SALADIN_LANGUAGES)
    list(APPEND SALADIN_TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/saladin_${lang}.ts")
endforeach()

find_package(Qt6 QUIET COMPONENTS LinguistTools)

if(TARGET Qt6::lrelease)
    # Recollect the user-visible strings into the .ts catalogs. The sources
    # (and include paths) are taken from the "saladin" target itself, so no
    # separate file list has to be maintained here.
    qt_add_lupdate(
        SOURCE_TARGETS saladin
        TS_FILES ${SALADIN_TS_FILES}
        LUPDATE_TARGET saladin_lupdate
        OPTIONS -no-obsolete)

    # Compile the .ts sources into .qm files in the build tree; the
    # "saladin_lrelease" target is part of ALL and "release_translations"
    # rebuilds only the translations.
    qt_add_lrelease(
        TS_FILES ${SALADIN_TS_FILES}
        QM_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/translations"
        QM_FILES_OUTPUT_VARIABLE SALADIN_QM_FILES
        LRELEASE_TARGET saladin_lrelease)

    # lrelease writes the catalogs into the build tree, but the runtime
    # language files are deployed next to the executable as
    # "<executable dir>/translations" (see "Qt runtime deployment"), which with
    # this project's in-source-tree output layout is served from the
    # translations/ directory below the source tree. Copy the generated
    # catalogs back so the deployed copy is never made from a stale .qm file.
    add_custom_target(saladin_sync_translations ALL
        COMMAND "${CMAKE_COMMAND}"
            "-DQM_BUILD_DIR=${CMAKE_BINARY_DIR}/translations"
            "-DQM_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/translations"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SyncTranslations.cmake"
        COMMENT "Syncing translation catalogs to the source tree"
        VERBATIM)
    add_dependencies(saladin_sync_translations saladin_lrelease)
else()
    # No Linguist tools available - fall back to the .qm files in the source tree.
    set(SALADIN_QM_FILES "")
    foreach(lang IN LISTS SALADIN_LANGUAGES)
        set(qm "${CMAKE_CURRENT_SOURCE_DIR}/translations/saladin_${lang}.qm")
        if(EXISTS "${qm}")
            list(APPEND SALADIN_QM_FILES "${qm}")
        else()
            message(WARNING "Missing translation: ${qm}")
        endif()
    endforeach()
endif()

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/locale.ini"
    ${SALADIN_QM_FILES}
    DESTINATION translations)
