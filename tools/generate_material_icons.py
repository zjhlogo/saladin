#!/usr/bin/env python3
"""Regenerate the Material Symbols action icons bundled with Saladin.

qt-material only ships a stylesheet plus the SVG *widget indicators* (check
boxes, arrows, splitters); it has no toolbar/action icon set. Saladin therefore
uses Google's Material Symbols (Outlined, weight 400), the icon language
qt-material's Material Design look is based on.

This script downloads the 24px outlined symbols, strips them down to a single
monochrome path (the runtime tints them with the active theme's text color, see
src/utils/iconloader.cpp) and rewrites src/icons/material/material.qrc.

The generated files are committed to the repository, so neither CMake nor a
normal build needs Python or a network connection. Run this script only when
icons or the mapping below have to change:

    python tools/generate_material_icons.py            # network download
    python tools/generate_material_icons.py --cache DIR  # reuse downloaded svg

The mapping keys are Saladin's icon names (IconLoader::icon("...") / the PNG
file names in src/icons); the values are Material Symbols names from
https://github.com/google/material-design-icons (Apache-2.0, see LICENSE in
src/icons/material).
"""

from __future__ import annotations

import argparse
import re
import sys
import urllib.error
import urllib.request
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = REPO_ROOT / "src" / "icons" / "material"

RESOURCE_PREFIX = "/icons/material"

RAW_BASE = "https://raw.githubusercontent.com/google/material-design-icons/master/symbols/web/{name}/materialsymbolsoutlined/{name}_24px.svg"

# Saladin icon name -> Material Symbols name. "saladin" (the application logo)
# has no Material counterpart and keeps its existing PNG files.
MAPPING = {
    "about": "info",
    "arrow-down": "keyboard_arrow_down",
    "arrow-hide": "keyboard_arrow_up",
    "arrow-left": "arrow_back",
    "arrow-right": "arrow_forward",
    "arrow-top": "vertical_align_top",
    "arrow-up": "arrow_upward",
    "black-background": "contrast",
    "bookmark": "bookmark",
    "browse": "folder",
    "calculate": "calculate",
    "clone": "file_copy",
    "close": "close",
    "compare": "difference",
    "configure": "settings",
    "copy": "content_copy",
    "copy-names": "copy_all",
    "delete": "delete_forever",
    "edit": "edit",
    "edit-copy": "content_copy",
    "edit-cut": "content_cut",
    "edit-paste": "content_paste",
    "encoding": "translate",
    "explore": "open_in_new",
    "file-new": "note_add",
    "find": "manage_search",
    "find-next": "keyboard_arrow_down",
    "find-previous": "keyboard_arrow_up",
    "find-text": "search",
    "folder-compare": "folder_copy",
    "folder-new": "create_new_folder",
    "ftp": "cloud",
    "fullscreen": "fullscreen",
    "gear": "build",
    "goto": "my_location",
    "help": "help",
    "hex-mode": "code",
    "history": "history",
    "home": "home",
    "info": "info",
    "license": "license",
    "line-numbers": "format_list_numbered",
    "move": "drive_file_move",
    "multi-rename": "edit_note",
    "open": "folder_open",
    "pack": "folder_zip",
    "pane-same": "vertical_split",
    "pane-swap": "swap_horiz",
    "refresh": "refresh",
    "rename": "drive_file_rename_outline",
    "rotate-left": "rotate_left",
    "rotate-right": "rotate_right",
    "select": "filter_alt",
    "select-all": "select_all",
    "select-invert": "flip",
    "terminal": "terminal",
    "trashcan": "delete",
    "type-binary": "data_object",
    "type-image": "image",
    "type-text": "article",
    "unselect": "filter_alt_off",
    "unselect-all": "deselect",
    "view": "visibility",
    "view-hidden": "visibility_off",
    "view-selected": "slideshow",
    "warning": "warning",
    "web": "language",
    "word-wrap": "wrap_text",
    "zoom-fit": "fit_screen",
    "zoom-in": "zoom_in",
    "zoom-orig": "1x_mobiledata",
    "zoom-out": "zoom_out",
}

VIEW_BOX_RE = re.compile(r'viewBox="([^"]+)"')
SIZE_RE = re.compile(r'width="([\d.]+)"')
BODY_RE = re.compile(r"<svg[^>]*>(.*)</svg>", re.DOTALL)


def download(name: str, cache_dir: Path | None) -> str:
    if cache_dir is not None:
        cached = cache_dir / f"{name}.svg"
        if cached.is_file():
            return cached.read_text(encoding="utf-8")

    url = RAW_BASE.format(name=name)
    with urllib.request.urlopen(url, timeout=60) as response:
        text = response.read().decode("utf-8")

    if cache_dir is not None:
        cache_dir.mkdir(parents=True, exist_ok=True)
        (cache_dir / f"{name}.svg").write_text(text, encoding="utf-8")

    return text


def normalize(text: str, symbol: str) -> str:
    """Keep the viewBox and the path data, force a single black fill.

    The runtime recolors the icon, so the fill color here only has to be
    opaque; ``fill`` on the root element is inherited by every path.
    """

    view_box = VIEW_BOX_RE.search(text)
    body = BODY_RE.search(text)
    if body is None:
        raise ValueError(f"unexpected svg structure for '{symbol}'")

    if view_box is not None:
        box = view_box.group(1)
    else:
        # A few symbols ship without a viewBox; their width/height is the box.
        size = SIZE_RE.search(text)
        edge = size.group(1) if size is not None else "24"
        box = f"0 0 {edge} {edge}"

    content = " ".join(body.group(1).split())
    return (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="%s" fill="#000000">'
        "%s</svg>\n" % (box, content)
    )


def write_qrc(names: list[str]) -> None:
    lines = ['<RCC version="1.0">', f'<qresource prefix="{RESOURCE_PREFIX}">']
    lines += [f"  <file>{name}.svg</file>" for name in names]
    lines += ["</qresource>", "</RCC>", ""]
    (OUTPUT_DIR / "material.qrc").write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cache", type=Path, help="directory holding/receiving the downloaded Material Symbols svg files")
    args = parser.parse_args()

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    symbols = sorted(set(MAPPING.values()))
    downloaded: dict[str, str] = {}
    failures: list[str] = []

    def fetch(symbol: str) -> None:
        try:
            downloaded[symbol] = download(symbol, args.cache)
        except (urllib.error.URLError, urllib.error.HTTPError) as error:
            failures.append(f"{symbol}: {error}")

    with ThreadPoolExecutor(16) as pool:
        list(pool.map(fetch, symbols))

    if failures:
        for failure in sorted(failures):
            print(f"error: {failure}", file=sys.stderr)
        return 1

    for app_name, symbol in sorted(MAPPING.items()):
        (OUTPUT_DIR / f"{app_name}.svg").write_text(normalize(downloaded[symbol], symbol), encoding="utf-8")

    names = sorted(MAPPING)
    write_qrc(names)

    # Drop stale generated files that are no longer part of the mapping.
    for stale in OUTPUT_DIR.glob("*.svg"):
        if stale.stem not in MAPPING:
            stale.unlink()

    print(f"wrote {len(names)} icons and material.qrc to {OUTPUT_DIR}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())