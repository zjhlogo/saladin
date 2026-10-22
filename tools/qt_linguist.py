#!/usr/bin/env python3
"""Shared helper for locating the Qt Linguist command line tools.

The translation helper scripts (update_translations.py,
release_translations.py) need the lupdate / lrelease executables of a Qt
installation. They are looked up in this order:

1. the directory passed as --qt-bin,
2. the CMakeCache.txt of the build/ directory (it records the Qt installation
   the project was last configured with),
3. the directories listed in the PATH,
4. the CMAKE_PREFIX_PATH environment variable,
5. the usual Windows Qt installer layouts (C:/Qt/<version>/<kit>/bin and
   ~/Qt/<version>/<kit>/bin).

Nothing here is needed by a normal build of Saladin; the scripts are only run
on demand when the translation catalogs have to change.
"""

from __future__ import annotations

import os
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]

_BUILD_CACHE = REPO_ROOT / "build" / "CMakeCache.txt"

# Qt 6 installs the tools as "lupdate"/"lrelease" (on Windows with ".exe");
# early Qt 6 releases shipped versioned names ("lupdate6") as well.
_TOOL_STEMS = ("{name}6", "{name}")
_TOOL_SUFFIXES = (".exe", "")


def _existing_tool(directory: Path, name: str) -> Path | None:
    if not directory.is_dir():
        return None
    for stem in _TOOL_STEMS:
        for suffix in _TOOL_SUFFIXES:
            candidate = directory / (stem.format(name=name) + suffix)
            if candidate.is_file():
                return candidate.resolve()
    return None


def _split_list(value: str) -> list[str]:
    return [entry for entry in re.split(r"[;]", value) if entry]


def _qt_dirs_from_build_cache() -> list[Path]:
    """Qt binary directories recorded by a previous CMake configure."""
    dirs: list[Path] = []
    try:
        cache = _BUILD_CACHE.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return dirs

    # Qt6_DIR points to <prefix>/lib/cmake/Qt6[LinguistTools].
    for match in re.finditer(r"^(Qt6_DIR|Qt6LinguistTools_DIR):PATH=(.+)$", cache, re.MULTILINE):
        dirs.append((Path(match.group(2)) / ".." / ".." / ".." / "bin").resolve())
    for match in re.finditer(r"^CMAKE_PREFIX_PATH:[^=]*=(.*)$", cache, re.MULTILINE):
        dirs += [Path(entry) / "bin" for entry in _split_list(match.group(1).strip())]
    return dirs


def _qt_dirs_from_environment() -> list[Path]:
    dirs = [Path(entry) for entry in os.environ.get("PATH", "").split(os.pathsep) if entry]
    for entry in _split_list(os.environ.get("CMAKE_PREFIX_PATH", "")):
        dirs.append(Path(entry) / "bin")
    qt_bin = os.environ.get("QT_BIN_DIR")
    if qt_bin:
        dirs.append(Path(qt_bin))
    return dirs


def _qt_dirs_from_common_layout() -> list[Path]:
    dirs: list[Path] = []
    for qt_root in (Path("C:/Qt"), Path.home() / "Qt"):
        if qt_root.is_dir():
            dirs += sorted(qt_root.glob("*/msvc*/bin"))
            dirs += sorted(qt_root.glob("*/mingw*/bin"))
    return dirs


def find_qt_tool(name: str, qt_bin: Path | None = None) -> Path | None:
    """Locate a Qt command line tool ("lupdate", "lrelease", ...).

    Returns the resolved executable path, or None when no Qt installation
    with that tool was found (the caller reports the error).
    """
    directories: list[Path] = []
    if qt_bin is not None:
        directories.append(qt_bin)
    directories += _qt_dirs_from_build_cache()
    directories += _qt_dirs_from_environment()
    directories += _qt_dirs_from_common_layout()

    seen: set[str] = set()
    for directory in directories:
        key = str(directory).casefold()
        if key in seen:
            continue
        seen.add(key)
        tool = _existing_tool(directory, name)
        if tool is not None:
            return tool
    return None


def qt_prefix(tool: Path) -> Path:
    """Qt installation prefix of a tool found in <prefix>/bin."""
    return tool.parent.parent
