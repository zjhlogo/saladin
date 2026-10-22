#!/usr/bin/env python3
"""Compile the .ts catalogs into the .qm files loaded at runtime.

Replaces the former CMake targets "saladin_lrelease" / "release_translations"
and the "saladin_sync_translations" copy step (removed from the top-level
CMakeLists.txt). The .qm files are written straight into the source tree:

- translations/saladin_*.ts  -> translations/saladin_*.qm (application), and
- plugins/<dir>/translations/<id>_*.ts -> .qm next to them (plugins whose
  metadata.json opts in, see translation_catalogs.py).

After compiling, the catalogs are deployed to the in-source-tree output
directories, like the build-time deployment does for the application runtime:

- release/translations and debug/translations        (application .qm)
- release/plugins/<plugin folder>/translations and the same below debug/
  (plugin .qm, per plugin: "<output>/plugins/<target>/translations" is the
  directory the plugin loads them from at runtime, see plugins/ftp)

Pass --no-deploy to only (re)compile the source-tree catalogs. lrelease is
located automatically (see qt_linguist.py for the search order); --qt-bin
picks a Qt installation explicitly. Run this script after the .ts files have
changed (by hand with Qt Linguist, or after tools/update_translations.py has
recollected the strings).
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

from qt_linguist import REPO_ROOT, find_qt_tool
from translation_catalogs import (APP_TRANSLATIONS_DIR, APP_TS_PREFIX,
                                  discover_plugins)

OUTPUT_CONFIG_DIRS = ("release", "debug")


def compile_catalog(lrelease: Path, ts_file: Path) -> Path | None:
    """Compile one .ts file into a .qm next to it; None on failure."""
    qm_file = ts_file.with_suffix(".qm")
    print(f"Compiling {ts_file.relative_to(REPO_ROOT)} -> {qm_file.name}")
    sys.stdout.flush()
    result = subprocess.run([str(lrelease), str(ts_file), "-qm", str(qm_file)])
    return qm_file if result.returncode == 0 else None


def collect_group(lrelease: Path, ts_dir: Path, prefix: str) -> list[Path]:
    """Compile all <prefix>_*.ts catalogs in ts_dir; returns the .qm files."""
    ts_files = sorted(ts_dir.glob(f"{prefix}_*.ts"))
    if not ts_files:
        print(f"warning: no {prefix}_*.ts catalogs in {ts_dir.relative_to(REPO_ROOT)}",
              file=sys.stderr)
        return []
    qm_files = []
    for ts_file in ts_files:
        qm_file = compile_catalog(lrelease, ts_file)
        if qm_file is not None:
            qm_files.append(qm_file)
    return qm_files


def deploy(qm_files: list[Path], prefix: str, target_dir: Path,
           created: set[Path]) -> None:
    """Copy the .qm files into an output directory and prune stale ones.

    Only catalogs with the given prefix (<prefix>_*.qm) are pruned, so the
    per-plugin translations directory never loses other plugins' files; a
    catalog is removed when its source .ts file no longer exists.
    """
    if not qm_files:
        return
    if target_dir not in created:
        target_dir.mkdir(parents=True, exist_ok=True)
        created.add(target_dir)

    copied: set[Path] = set()
    for qm_file in qm_files:
        target = target_dir / qm_file.name
        shutil.copy2(qm_file, target)
        copied.add(target)

    for stale in sorted(target_dir.glob(f"{prefix}_*.qm")):
        if stale not in copied:
            print(f"Pruned stale {stale.relative_to(REPO_ROOT)}")
            stale.unlink()

    names = ", ".join(qm_file.name for qm_file in qm_files)
    print(f"Deployed {names} -> {target_dir.relative_to(REPO_ROOT)}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--qt-bin", type=Path, metavar="DIR",
                        help="Qt binary directory holding lrelease (default: auto-detect)")
    parser.add_argument("--no-deploy", action="store_true",
                        help="only compile the source-tree catalogs, skip the "
                             "release/debug output directories")
    args = parser.parse_args()

    lrelease = find_qt_tool("lrelease", args.qt_bin)
    if lrelease is None:
        print("error: lrelease not found; pass --qt-bin <Qt>/bin", file=sys.stderr)
        return 1

    print(f"Using {lrelease}")

    failed = False

    app_qm_files = collect_group(lrelease, APP_TRANSLATIONS_DIR, APP_TS_PREFIX)
    failed = failed or (not app_qm_files)

    plugin_groups: list[tuple[object, list[Path]]] = []
    for plugin in discover_plugins():
        qm_files = collect_group(lrelease, plugin.translations_dir, plugin.plugin_id)
        failed = failed or bool(plugin.ts_files() and not qm_files)
        if qm_files:
            plugin_groups.append((plugin, qm_files))

    if not args.no_deploy:
        created: set[Path] = set()
        for config in OUTPUT_CONFIG_DIRS:
            config_dir = REPO_ROOT / config
            if not config_dir.is_dir():
                continue  # not built yet in this configuration
            deploy(app_qm_files, APP_TS_PREFIX, config_dir / "translations", created)
            for plugin, qm_files in plugin_groups:
                deploy(qm_files, plugin.plugin_id,
                       config_dir / "plugins" / plugin.runtime_dir_name / "translations",
                       created)

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
