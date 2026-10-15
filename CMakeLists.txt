#############################################################################
# Saladin - dual-pane file manager for Windows
#
# CMake build definition (replaces the former qmake project files).
#
# Quick start (Qt 6 + MinGW):
#
#   set PATH=C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%
#   cmake -S . -B build -G "MinGW Makefiles" ^
#         -DCMAKE_BUILD_TYPE=Release ^
#         -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64 ^
#         -DCMAKE_INSTALL_PREFIX=%CD%/dist
#   cmake --build build --parallel
#   cmake --install build
#
# Quick start (Qt 6 + MSVC):
#
#   rmdir /s /q build
#   cmake -S . -B build -G "Visual Studio 18 2026" -A x64 ^
#         -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
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
    Core Gui Widgets Network Xml
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
# Translations
# ---------------------------------------------------------------------------

set(SALADIN_LANGUAGES de es hu pl pt_BR ru)

set(SALADIN_TS_FILES "")
foreach(lang IN LISTS SALADIN_LANGUAGES)
    list(APPEND SALADIN_TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/saladin_${lang}.ts")
endforeach()

find_package(Qt6 QUIET COMPONENTS LinguistTools)

if(TARGET Qt6::lrelease)
    # Compile the .ts sources into .qm files in the build tree; the
    # "saladin_lrelease" target is part of ALL and "release_translations"
    # rebuilds only the translations.
    qt_add_lrelease(
        TS_FILES ${SALADIN_TS_FILES}
        QM_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/translations"
        QM_FILES_OUTPUT_VARIABLE SALADIN_QM_FILES
        LRELEASE_TARGET saladin_lrelease)
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

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

add_subdirectory(src)
