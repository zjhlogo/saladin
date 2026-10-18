#!/usr/bin/env python3
"""Regenerate the Qt-Material assets bundled with Saladin.

The generated files (QSS, SVG indicator icons and themes.qrc) are committed to
the repository, so neither CMake nor a normal build needs Python. Run this
script only when the bundled themes have to be created or updated:

    python -m pip install --target tmp/qtmaterial "qt-material==2.17" "PySide6-Essentials==6.12.0"
    $env:PYTHONPATH = "$PWD\\tmp\\qtmaterial"    # PowerShell
    python tools/generate_qt_material_theme.py

See src/themes/qt-material/README.md for details.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = REPO_ROOT / "src" / "themes" / "qt-material"

# Resource prefix used by both the QSS icon URLs and themes.qrc.
RESOURCE_PREFIX = "/themes/qt-material"

# Long-lived theme pairs used by ThemeHelper: Dark -> dark_teal, Light -> light_teal.
#
# "primary_text_color" is written into the vendored theme XML. qt-material uses
# it for text drawn on the primary/accent color (selected menu entries, combo
# box items, focused item views). The PyPI 2.17 data uses white there, which is
# unreadable on the bright teal accent, so a contrast-safe value is pinned.
THEMES = (
    {
        "name": "dark_teal",
        "source": "dark_teal.xml",
        "invert_secondary": False,
        "primary_text_color": "#000000",
    },
    {
        "name": "light_teal",
        "source": "light_teal.xml",
        "invert_secondary": True,
        "primary_text_color": "#3c3c3c",
    },
)

ICON_FOLDERS = ("active", "disabled", "primary")


def clean_qss(text: str) -> str:
    """Drop declarations that Qt either ignores or that would break Saladin.

    * global font declarations would override the viewer fonts configured in
      the settings dialog,
    * ``text-transform`` and ``unset`` values are not supported by Qt and only
      produce stylesheet warnings.
    """

    kept: list[str] = []

    for line in text.splitlines():
        stripped = line.strip()

        if re.match(r"^(font-family|font-size|line-height)\s*:", stripped):
            continue
        if stripped.startswith("text-transform:"):
            continue
        if "unset" in stripped:
            continue
        # Drop empty declarations such as ``font-size: ;``.
        if re.match(r"^[a-z-]+\s*:\s*;\s*$", stripped):
            continue

        kept.append(line)

    # Collapse the blank lines left behind by the removed declarations.
    return re.sub(r"\n{3,}", "\n\n", "\n".join(kept)).rstrip() + "\n"


def write_qrc() -> None:
    entries: list[str] = ["dark_teal.qss", "light_teal.qss", "saladin.qss"]

    for theme in THEMES:
        entries.append(f"themes/{theme['source']}")

    for theme in THEMES:
        for folder in ICON_FOLDERS:
            directory = OUTPUT_DIR / theme["name"] / folder
            if not directory.is_dir():
                raise SystemExit(f"missing generated icon directory: {directory}")
            for icon in sorted(directory.glob("*.svg")):
                entries.append(f"{theme['name']}/{folder}/{icon.name}")

    lines = ["<RCC>", f'  <qresource prefix="{RESOURCE_PREFIX}">']
    lines.extend(f"    <file>{entry}</file>" for entry in entries)
    lines.extend(["  </qresource>", "</RCC>", ""])

    (OUTPUT_DIR / "themes.qrc").write_text("\n".join(lines), encoding="utf-8")


def vendored_theme_xml(package_dir: Path, theme: dict) -> Path:
    """Copy a qt-material theme XML, pinning the on-accent text color."""

    text = (package_dir / "themes" / theme["source"]).read_text(encoding="utf-8")
    text = re.sub(
        r'(<color name="primaryTextColor">)[^<]*(</color>)',
        lambda match: match.group(1) + theme["primary_text_color"] + match.group(2),
        text,
        count=1,
    )

    target = OUTPUT_DIR / "themes" / theme["source"]
    target.write_text(text, encoding="utf-8")
    return target


def main() -> int:
    # qt-material has to be imported after a Qt binding; a QApplication is
    # created because the exporter renders the icons with the Qt paint engine.
    from PySide6.QtWidgets import QApplication

    QApplication.instance() or QApplication([])

    import qt_material
    from qt_material import export_theme

    package_dir = Path(qt_material.__file__).resolve().parent
    themes_dir = OUTPUT_DIR / "themes"
    themes_dir.mkdir(parents=True, exist_ok=True)

    for theme in THEMES:
        name = theme["name"]
        icon_dir = OUTPUT_DIR / name
        qss_path = OUTPUT_DIR / f"{name}.qss"
        theme_xml = vendored_theme_xml(package_dir, theme)

        print(f"[qt-material] exporting {name} ...")
        export_theme(
            theme=str(theme_xml),
            qss=str(qss_path),
            rcc=None,
            invert_secondary=theme["invert_secondary"],
            extra={},
            output=str(icon_dir),
            prefix=f":{RESOURCE_PREFIX}/{name}/",
        )

        qss_path.write_text(clean_qss(qss_path.read_text(encoding="utf-8")), encoding="utf-8")

        text = qss_path.read_text(encoding="utf-8")
        if "icon:/" in text:
            raise SystemExit(f"{qss_path} still contains 'icon:/' URLs")
        if re.search(r"^\s*(font-family|font-size|line-height)\s*:", text, re.MULTILINE):
            raise SystemExit(f"{qss_path} still contains font declarations")

    write_qrc()

    print(f"[qt-material] assets written to {OUTPUT_DIR.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())