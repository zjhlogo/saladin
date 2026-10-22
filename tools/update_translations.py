#!/usr/bin/env python3
"""Recollect the user-visible strings from the sources into the .ts catalogs.

Replaces the former CMake targets "saladin_lupdate" / "update_translations"
(removed from the top-level CMakeLists.txt). Runs Qt's lupdate over

- the application sources (all .cpp/.h below src/) into the
  translations/saladin_*.ts catalogs, and
- every plugin that maintains its own catalogs (metadata.json
  "translations": true, see translation_catalogs.py) into
  plugins/<dir>/translations/<id>_*.ts.

Existing translations are preserved; only the collected strings and file
locations are refreshed. For plugins, catalogs for the application's
languages are created on the fly (e.g. plugins/ftp/translations/ftp_zh_CN.ts).

    python tools/update_translations.py
    python tools/update_translations.py --qt-bin C:/Qt/6.12.0/msvc2022_64/bin
    python tools/update_translations.py --keep-obsolete

Strings removed from the sources are dropped from the catalogs (-no-obsolete,
like the former CMake target); pass --keep-obsolete to keep them as "vanished"
entries instead. lupdate is located automatically (see qt_linguist.py for the
search order); --qt-bin picks a Qt installation explicitly.

A normal build of Saladin does not need this script - the catalogs only have
to be refreshed after user-visible strings in the sources have changed.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

from qt_linguist import REPO_ROOT, find_qt_tool, qt_prefix
from translation_catalogs import (APP_TRANSLATIONS_DIR, APP_TS_PREFIX,
                                  PluginCatalog, app_languages,
                                  discover_plugins)

SOURCE_DIR = REPO_ROOT / "src"

# Modules linked by the application and the plugins. Their include paths are
# handed to lupdate so that its C++ parser resolves the Qt headers, including
# the private ones.
QT_MODULES = ("QtCore", "QtGui", "QtWidgets", "QtNetwork", "QtXml", "QtSvg")

_VERSIONED_DIR = re.compile(r"\d+\.\d+(\.\d+)?$")


@dataclass
class CatalogGroup:
    """One lupdate run: a source set feeding same-prefixed .ts catalogs."""

    prefix: str                      # .ts file prefix ("saladin", "ftp", ...)
    ts_dir: Path                     # directory holding the catalogs
    sources: list[Path]
    include_paths: list[Path]        # first entry is the source root
    languages: list[str]
    label: str = field(default="")

    def __post_init__(self) -> None:
        if not self.label:
            self.label = f"{self.prefix}_* in {self.ts_dir.relative_to(REPO_ROOT)}"

    def ts_files(self) -> list[Path]:
        return [self.ts_dir / f"{self.prefix}_{language}.ts"
                for language in self.languages]


def collect_sources(directory: Path) -> list[Path]:
    """Every source/header below directory (mirrors the CMake target lists)."""
    return sorted({*directory.rglob("*.cpp"), *directory.rglob("*.h")})


def qt_include_paths(prefix: Path) -> list[Path]:
    include = prefix / "include"
    paths = [include]
    for module in QT_MODULES:
        module_dir = include / module
        paths.append(module_dir)
        # Private headers live in <include>/<Module>/<version>[/<Module>].
        for version_dir in sorted(include.glob(f"{module}/*")):
            if version_dir.is_dir() and _VERSIONED_DIR.fullmatch(version_dir.name):
                paths.append(version_dir)
                paths.append(version_dir / module)
    mkspecs = prefix / "mkspecs" / "win32-msvc"
    if mkspecs.is_dir():
        paths.append(mkspecs)
    return paths


def unique_existing_dirs(paths: list[Path]) -> list[str]:
    result: list[str] = []
    seen: set[str] = set()
    for path in paths:
        key = str(path).casefold()
        if path.is_dir() and key not in seen:
            seen.add(key)
            result.append(str(path))
    return result


def application_group(qt_includes: list[Path]) -> CatalogGroup | None:
    languages = app_languages()
    if not languages:
        return None
    sources = collect_sources(SOURCE_DIR)
    if not sources:
        return None
    return CatalogGroup(APP_TS_PREFIX, APP_TRANSLATIONS_DIR, sources,
                        [SOURCE_DIR] + qt_includes, languages,
                        label=f"application ({APP_TS_PREFIX}_*)")


def plugin_group(plugin: PluginCatalog, languages: list[str],
                 qt_includes: list[Path]) -> CatalogGroup | None:
    sources = collect_sources(plugin.directory)
    if not sources:
        return None
    return CatalogGroup(plugin.plugin_id, plugin.translations_dir, sources,
                        [plugin.directory, SOURCE_DIR] + qt_includes, languages)


def run_lupdate(lupdate: Path, group: CatalogGroup, keep_obsolete: bool) -> int:
    # lupdate project file - the same mechanism the removed qt_add_lupdate()
    # used: absolute paths, so the catalogs are independent of the directory
    # the script runs from. The temporary file is deleted afterwards.
    project = {
        "projectFile": str(REPO_ROOT / "CMakeLists.txt"),
        "includePaths": unique_existing_dirs(group.include_paths),
        "sources": [str(path) for path in group.sources],
        "translations": [str(path) for path in group.ts_files()],
    }

    descriptor, json_name = tempfile.mkstemp(prefix="saladin_lupdate_", suffix=".json")
    json_path = Path(json_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as file:
            json.dump(project, file, indent=2)

        command = [str(lupdate), "-project", str(json_path)]
        if not keep_obsolete:
            command.append("-no-obsolete")

        print(f"--- {group.label}")
        sys.stdout.flush()
        # Run in the catalog directory so lupdate stores the file locations
        # relative to it ("../src/..." / "../ftp_dialog.cpp"), like the
        # former CMake target did.
        result = subprocess.run(command, cwd=group.ts_dir)
    finally:
        json_path.unlink(missing_ok=True)
    return result.returncode


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--qt-bin", type=Path, metavar="DIR",
                        help="Qt binary directory holding lupdate (default: auto-detect)")
    parser.add_argument("--keep-obsolete", action="store_true",
                        help='keep entries for strings removed from the sources ("vanished")')
    args = parser.parse_args()

    lupdate = find_qt_tool("lupdate", args.qt_bin)
    if lupdate is None:
        print("error: lupdate not found; pass --qt-bin <Qt>/bin", file=sys.stderr)
        return 1

    print(f"Using {lupdate}")

    groups: list[CatalogGroup] = []
    qt_includes = qt_include_paths(qt_prefix(lupdate))

    application = application_group(qt_includes)
    if application is not None:
        groups.append(application)
    else:
        print("warning: no application sources or no saladin_*.ts catalogs found",
              file=sys.stderr)

    for plugin in discover_plugins():
        # Maintain the application's languages in every plugin, plus any
        # language the plugin already has catalogs for.
        languages = list(dict.fromkeys(app_languages() + plugin.languages()))
        if not languages:
            print(f"warning: no languages to collect for plugin '{plugin.plugin_id}'",
                  file=sys.stderr)
            continue
        group = plugin_group(plugin, languages, qt_includes)
        if group is None:
            print(f"warning: no sources below {plugin.directory}", file=sys.stderr)
            continue
        group.ts_dir.mkdir(parents=True, exist_ok=True)
        groups.append(group)

    failed = False
    for group in groups:
        if run_lupdate(lupdate, group, args.keep_obsolete) != 0:
            failed = True
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
