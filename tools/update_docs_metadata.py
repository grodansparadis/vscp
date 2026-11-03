#!/usr/bin/env python3
"""Refresh docs metadata for the generated documentation site.

By default this updates docs/variables.xml with the current timestamp and a
version string in the format YY.MM.<commit-count>. With --prepare-commit, it
also updates version.h using the count of the metadata commit being prepared.
"""

from __future__ import annotations

import argparse
import re
import subprocess
from datetime import datetime
from pathlib import Path
import xml.etree.ElementTree as ET


REPO_ROOT = Path(__file__).resolve().parents[1]
VARIABLES_PATH = REPO_ROOT / "docs" / "variables.xml"
VERSION_HEADER_PATH = REPO_ROOT / "src" / "vscp" / "common" / "version.h"


def _commit_count(*, strict: bool = False) -> int:
    """Return the number of commits reachable from HEAD."""
    try:
        result = subprocess.run(
            ["git", "-C", str(REPO_ROOT), "rev-list", "--count", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        )
        return int(result.stdout.strip())
    except (OSError, ValueError, subprocess.CalledProcessError):
        if strict:
            raise
        return 0


def _update_version_header(header: str, now: datetime, commit_count: int) -> str:
    """Replace the existing version definitions without moving their lines."""
    version = f"{now:%y}.{now:%m}.{commit_count}"
    definitions = {
        "VSCPD_MAJOR_VERSION": str(now.year % 100),
        "VSCPD_MINOR_VERSION": str(now.month),
        "VSCPD_PATCH_VERSION": str(commit_count),
        "VSCPD_DISPLAY_VERSION": f'"{version}"',
    }

    for name, value in definitions.items():
        declaration = rf"(?m)^[ \t]*#[ \t]*define[ \t]+{name}(?![a-zA-Z0-9_])"
        if len(re.findall(declaration, header)) != 1:
            raise ValueError(f"Expected exactly one valid {name} definition in version.h")
        pattern = (
            rf'(?m)^([ \t]*#[ \t]*define[ \t]+{name}[ \t]+)'
            r'(?:"[^"\r\n]*"|[0-9]+)(?=[ \t]*(?://|/\*|$))'
        )
        header, count = re.subn(pattern, lambda match: match.group(1) + value, header)
        if count != 1:
            raise ValueError(f"Expected exactly one valid {name} definition in version.h")
    return header


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--prepare-commit",
        action="store_true",
        help="update version.h and include the metadata commit in the version count",
    )
    args = parser.parse_args()

    if not VARIABLES_PATH.exists():
        raise FileNotFoundError(f"Missing variables file: {VARIABLES_PATH}")

    tree = ET.parse(VARIABLES_PATH)
    root = tree.getroot()

    now = datetime.now()
    creation_time = now.strftime("%Y-%m-%d %H:%M")
    commit_count = _commit_count(strict=args.prepare_commit)
    if args.prepare_commit:
        if commit_count < 1:
            raise ValueError("A valid Git commit count is required to prepare metadata")
        commit_count += 1
    version = f"{now:%y}.{now:%m}.{commit_count}"

    creation_node = root.find("creation-time")
    version_node = root.find("document-version")

    if creation_node is None or version_node is None:
        raise ValueError("variables.xml must contain creation-time and document-version elements")

    header = None
    if args.prepare_commit:
        header = _update_version_header(
            VERSION_HEADER_PATH.read_text(encoding="utf-8"), now, commit_count
        )

    creation_node.text = creation_time
    version_node.text = version

    tree.write(VARIABLES_PATH, encoding="utf-8", xml_declaration=False)
    if header is not None:
        VERSION_HEADER_PATH.write_text(header, encoding="utf-8")
    print(f"Updated docs metadata: {creation_time} / {version_node.text}")


if __name__ == "__main__":
    main()
