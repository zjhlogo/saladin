#!/usr/bin/env python3
"""Shared description of the catalogs maintained by the translation scripts.

Besides the application catalogs in translations/ (file prefix "saladin"),
the scripts also handle plugin catalogs. A plugin opts in by setting
"translations": true in its metadata.json (or by already owning a
translations/ directory). Its catalogs live in plugins/<dir>/translations/
and are named <plugin id>_<language>.ts, where the plugin id is the
metadata.json "name" - the same id the plugin passes to QTranslator::load()
(e.g. "ftp" in plugins/ftp/ftp_plugin.cpp).

The host ignores the "translations" key; it is only read by these scripts.
Plugins that do not opt in (like plugins/example, which keeps English text)
are not touched.
"""

from __future__ import annotations

import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path

from qt_linguist import REPO_ROOT

APP_TRANSLATIONS_DIR = REPO_ROOT / "translations"
APP_TS_PREFIX = "saladin"
PLUGINS_DIR = REPO_ROOT / "plugins"

_SAFE_ID = re.compile(r"[A-Za-z0-9_-]+")

# The runtime layout deploys every plugin into "<output>/plugins/<target>/",
# where <target> is the qt_add_plugin() target from the plugin's own
# CMakeLists.txt (the plugin folder is named after its DLL). The translation
# scripts mirror that layout when deploying the compiled catalogs.
_QT_ADD_PLUGIN = re.compile(r"\bqt_add_plugin\(\s*([A-Za-z0-9_.:-]+)")


@dataclass(frozen=True)
class PluginCatalog:
    """A plugin that maintains its own translation catalogs."""

    plugin_id: str          # metadata.json "name" - the QTranslator base name
    directory: Path         # plugins/<dir>
    translations_dir: Path  # plugins/<dir>/translations
    runtime_dir_name: str   # folder below <output>/plugins/ holding the DLL

    def ts_files(self) -> list[Path]:
        return sorted(self.translations_dir.glob(f"{self.plugin_id}_*.ts"))

    def languages(self) -> list[str]:
        prefix = self.plugin_id + "_"
        return [path.stem[len(prefix):] for path in self.ts_files()]


def _runtime_dir_name(directory: Path, plugin_id: str) -> str:
    """The plugin's runtime folder name: its qt_add_plugin() CMake target.

    Falls back to the plugin id (with a warning) when the plugin has no
    readable CMakeLists.txt, so the scripts keep working for out-of-tree
    catalogs without a build definition.
    """
    cmake_lists = directory / "CMakeLists.txt"

    try:
        text = cmake_lists.read_text(encoding="utf-8")
    except OSError:
        text = ""

    match = _QT_ADD_PLUGIN.search(text) if text else None

    if match:
        return match.group(1)

    print(f"warning: cannot read the qt_add_plugin() target from {cmake_lists}, "
          f"assuming the runtime folder is named '{plugin_id}'", file=sys.stderr)
    return plugin_id


def app_languages() -> list[str]:
    """Languages of the application catalogs (translations/saladin_*.ts)."""
    prefix = APP_TS_PREFIX + "_"
    return [path.stem[len(prefix):]
            for path in sorted(APP_TRANSLATIONS_DIR.glob(f"{APP_TS_PREFIX}_*.ts"))]


def discover_plugins() -> list[PluginCatalog]:
    """Plugins that maintain their own translation catalogs.

    A plugin participates when its metadata.json contains
    "translations": true, or when a translations/ directory with catalogs
    already exists next to it (covers plugins that ship .ts files without
    the metadata flag).
    """
    plugins: list[PluginCatalog] = []
    if not PLUGINS_DIR.is_dir():
        return plugins

    for directory in sorted(PLUGINS_DIR.iterdir()):
        if not directory.is_dir():
            continue

        metadata: dict = {}
        metadata_file = directory / "metadata.json"
        if metadata_file.is_file():
            try:
                metadata = json.loads(metadata_file.read_text(encoding="utf-8"))
            except (OSError, ValueError) as error:
                print(f"warning: ignoring {metadata_file}: {error}", file=sys.stderr)
                continue

        plugin_id = str(metadata.get("name") or directory.name)
        translations_dir = directory / "translations"

        opted_in = metadata.get("translations") is True or translations_dir.is_dir()
        if not opted_in:
            continue

        if not _SAFE_ID.fullmatch(plugin_id):
            print(f"warning: ignoring plugin id '{plugin_id}' in {metadata_file} "
                  "(not usable in file names)", file=sys.stderr)
            continue

        plugins.append(PluginCatalog(plugin_id, directory, translations_dir,
                                     _runtime_dir_name(directory, plugin_id)))

    return plugins
