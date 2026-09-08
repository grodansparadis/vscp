#!/usr/bin/env python3
"""Refresh docs metadata for the generated documentation site.

This updates docs/variables.xml with the current timestamp and a version string
in the format YY.MM.<commit-count>.
"""

from __future__ import annotations

import subprocess
from datetime import datetime
from pathlib import Path
import xml.etree.ElementTree as ET


REPO_ROOT = Path(__file__).resolve().parents[1]
VARIABLES_PATH = REPO_ROOT / "docs" / "variables.xml"


def _commit_count() -> int:
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
        return 0


def main() -> None:
    if not VARIABLES_PATH.exists():
        raise FileNotFoundError(f"Missing variables file: {VARIABLES_PATH}")

    tree = ET.parse(VARIABLES_PATH)
    root = tree.getroot()

    now = datetime.now()
    creation_time = now.strftime("%Y-%m-%d %H:%M")
    version = f"{now:%y}.{now:%m}.{_commit_count()}"

    creation_node = root.find("creation-time")
    version_node = root.find("document-version")

    if creation_node is None or version_node is None:
        raise ValueError("variables.xml must contain creation-time and document-version elements")

    creation_node.text = creation_time
    version_node.text = version

    tree.write(VARIABLES_PATH, encoding="utf-8", xml_declaration=False)
    print(f"Updated docs metadata: {creation_time} / {version_node.text}")


if __name__ == "__main__":
    main()
