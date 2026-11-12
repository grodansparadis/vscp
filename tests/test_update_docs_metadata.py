#!/usr/bin/env python3
"""Tests for the documentation and version metadata updater."""

import contextlib
import io
import os
from pathlib import Path
import re
import runpy
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET
from datetime import datetime


SOURCE_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_REVISION = 7


class UpdateMetadataTest(unittest.TestCase):
    def setUp(self):
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.root = Path(self.temporary_directory.name)
        (self.root / "tools").mkdir()
        (self.root / "docs").mkdir()
        (self.root / "src" / "vscp" / "common").mkdir(parents=True)
        self.script = self.root / "tools" / "update_docs_metadata.py"
        self.variables = self.root / "docs" / "variables.xml"
        self.header = self.root / "src" / "vscp" / "common" / "version.h"
        shutil.copyfile(SOURCE_ROOT / "tools" / "update_docs_metadata.py", self.script)
        shutil.copyfile(SOURCE_ROOT / "src" / "vscp" / "common" / "version.h", self.header)
        self.header.write_text(
            re.sub(
                r"(?m)(^#define VSCPD_REVISION_VERSION\s+)\d+$",
                rf"\g<1>{FIXTURE_REVISION}",
                self.header.read_text(encoding="utf-8"),
            ),
            encoding="utf-8",
        )
        self.variables.write_text(
            "<var><creation-time>old</creation-time>"
            "<document-version>old</document-version></var>\n",
            encoding="utf-8",
        )
        self.git("init", "-q")
        month_start = datetime.now().replace(
            day=1, hour=0, minute=0, second=0, microsecond=0
        )
        previous_month_timestamp = f"@{int(month_start.timestamp()) - 1} +0000"
        self.commit("Previous month commit", timestamp=previous_month_timestamp)
        self.commit("Current month commit", allow_empty=True)

    def git(self, *args, env=None):
        return subprocess.run(
            [
                "git", "-c", f"core.hooksPath={self.root / 'no-hooks'}",
                "-c", "commit.gpgsign=false", "-C", str(self.root), *args,
            ],
            check=True,
            capture_output=True,
            text=True,
            env=env,
        ).stdout.strip()

    def commit(self, message, *, allow_empty=False, timestamp=None):
        self.git("add", "docs/variables.xml", "src/vscp/common/version.h")
        options = ["--allow-empty"] if allow_empty else []
        env = os.environ.copy()
        if timestamp is not None:
            env["GIT_AUTHOR_DATE"] = timestamp
            env["GIT_COMMITTER_DATE"] = timestamp
        self.git("-c", "user.name=VSCP Test", "-c", "user.email=test@example.invalid",
                 "commit", *options, "-qm", message, env=env)

    def commits_this_month(self, now=None):
        now = now or datetime.now()
        return int(self.git(
            "rev-list", "--count", f"--since={now:%Y-%m-01}", "HEAD"
        ))

    def test_patch_count_resets_at_month_boundary(self):
        self.assertEqual(1, self.commits_this_month())

    def update(self, *args, env=None):
        return subprocess.run(
            [sys.executable, "-B", str(self.script), *args],
            capture_output=True,
            text=True,
            env=env,
        )

    def test_default_updates_only_documentation(self):
        original_header = self.header.read_text(encoding="utf-8")
        result = self.update()
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(original_header, self.header.read_text(encoding="utf-8"))
        version = ET.parse(self.variables).getroot().findtext("document-version")
        self.assertEqual(str(self.commits_this_month()), version.split(".")[3])
        self.assertEqual(str(FIXTURE_REVISION), version.split(".")[2])

    def test_default_preserves_git_unavailable_fallback(self):
        original_header = self.header.read_text(encoding="utf-8")
        env = os.environ.copy()
        env["PATH"] = str(self.root / "no-git")
        result = self.update(env=env)
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(original_header, self.header.read_text(encoding="utf-8"))
        version = ET.parse(self.variables).getroot().findtext("document-version")
        self.assertEqual("0", version.split(".")[3])
        self.assertEqual(str(FIXTURE_REVISION), version.split(".")[2])

    def test_prepared_commit_versions_match_resulting_head(self):
        original_lines = self.header.read_text(encoding="utf-8").splitlines()
        result = self.update("--prepare-commit")
        self.assertEqual(0, result.returncode, result.stderr)

        root = ET.parse(self.variables).getroot()
        created = datetime.strptime(root.findtext("creation-time"), "%Y-%m-%d %H:%M")
        version = root.findtext("document-version")
        revision = int(re.search(
            r"(?m)^#define VSCPD_REVISION_VERSION\s+([0-9]+)$",
            self.header.read_text(encoding="utf-8"),
        ).group(1))
        expected_patch = self.commits_this_month() + 1
        self.assertEqual(
            f"{created:%y}.{created:%m}.{revision}.{expected_patch}", version
        )

        header_lines = self.header.read_text(encoding="utf-8").splitlines()
        definitions = {
            "VSCPD_MAJOR_VERSION": str(created.year % 100),
            "VSCPD_MINOR_VERSION": str(created.month),
            "VSCPD_REVISION_VERSION": str(revision),
            "VSCPD_PATCH_VERSION": str(expected_patch),
            "VSCPD_DISPLAY_VERSION": f'"{version}"',
        }
        self.assertEqual(len(original_lines), len(header_lines))
        modified_indices = set()
        for name, value in definitions.items():
            pattern = re.compile(rf"^#define\s+{name}\s+(.+)$")
            original_indices = [i for i, line in enumerate(original_lines) if pattern.match(line)]
            updated_indices = [i for i, line in enumerate(header_lines) if pattern.match(line)]
            self.assertEqual(original_indices, updated_indices)
            self.assertEqual(1, len(updated_indices))
            self.assertEqual(value, pattern.match(header_lines[updated_indices[0]]).group(1))
            modified_indices.update(updated_indices)
        for i, (original, updated) in enumerate(zip(original_lines, header_lines)):
            if i not in modified_indices:
                self.assertEqual(original, updated)

        self.commit("Update version and documentation metadata")
        self.assertEqual(expected_patch, self.commits_this_month())

    def test_fast_forward_before_pushing_metadata(self):
        remote = self.root / "remote.git"
        other = self.root / "other"
        self.git("init", "--bare", "-q", str(remote))
        self.git("branch", "-M", "main")
        self.git("remote", "add", "origin", str(remote))
        self.git("push", "-u", "origin", "HEAD:refs/heads/main")
        self.git("clone", "-q", "--branch", "main", str(remote), str(other))
        self.git(
            "-C", str(other), "-c", "user.name=VSCP Test",
            "-c", "user.email=test@example.invalid",
            "commit", "--allow-empty", "-qm", "Concurrent push",
        )
        self.git("-C", str(other), "push", "origin", "HEAD:refs/heads/main")

        self.git("pull", "--ff-only", "origin", "main")
        result = self.update("--prepare-commit")
        self.assertEqual(0, result.returncode, result.stderr)
        self.commit("Update version and documentation metadata")
        self.git("push", "origin", "HEAD:refs/heads/main")

        remote_count = int(self.git(
            "--git-dir", str(remote), "rev-list", "--count",
            f"--since={datetime.now():%Y-%m-01}", "main",
        ))
        xml = self.git("--git-dir", str(remote), "show", "main:docs/variables.xml")
        version = ET.fromstring(xml).findtext("document-version")
        header = self.git(
            "--git-dir", str(remote), "show", "main:src/vscp/common/version.h"
        )
        self.assertEqual(remote_count, int(version.split(".")[3]))
        self.assertIn(f'#define VSCPD_DISPLAY_VERSION   "{version}"', header)

    def test_preseeded_metadata_still_needs_a_commit(self):
        module = runpy.run_path(str(self.script))
        now = datetime.now()
        expected_patch = self.commits_this_month(now) + 2
        revision = module["_manual_revision"](self.header.read_text(encoding="utf-8"))
        version = f"{now:%y}.{now:%m}.{revision}.{expected_patch}"
        header = module["_update_version_header"](
            self.header.read_text(encoding="utf-8"), now, expected_patch
        )
        self.header.write_text(header, encoding="utf-8")
        tree = ET.parse(self.variables)
        tree.getroot().find("creation-time").text = now.strftime("%Y-%m-%d %H:%M")
        tree.getroot().find("document-version").text = version
        tree.write(self.variables, encoding="utf-8", xml_declaration=False)
        self.commit("Preseed next version")

        class FrozenDateTime:
            @staticmethod
            def now():
                return now

        with patch.dict(module["main"].__globals__, datetime=FrozenDateTime), \
                patch.object(sys, "argv", [str(self.script), "--prepare-commit"]), \
                contextlib.redirect_stdout(io.StringIO()):
            module["main"]()

        self.git("diff", "--quiet", "--", "docs/variables.xml", "src/vscp/common/version.h")
        self.commit("Update version and documentation metadata", allow_empty=True)
        self.assertEqual(expected_patch, self.commits_this_month())

    def test_numeric_month_has_no_leading_zero(self):
        module = runpy.run_path(str(self.script))
        result = module["_update_version_header"](
            self.header.read_text(encoding="utf-8"), datetime(2026, 9, 1), 123
        )
        self.assertRegex(result, r"(?m)^#define VSCPD_MINOR_VERSION\s+9$")
        self.assertRegex(
            result, rf'(?m)^#define VSCPD_DISPLAY_VERSION\s+"26\.09\.{FIXTURE_REVISION}\.123"$'
        )

    def test_revision_is_manually_set_and_limited_to_one_byte(self):
        module = runpy.run_path(str(self.script))
        header = self.header.read_text(encoding="utf-8")
        self.assertEqual(FIXTURE_REVISION, module["_manual_revision"](header))
        self.assertEqual(0, module["_manual_revision"](
            re.sub(
                r"(?m)(^#define VSCPD_REVISION_VERSION\s+)\d+$",
                r"\g<1>0",
                header,
            )
        ))
        manual_revision = re.sub(
            r"(?m)(^#define VSCPD_REVISION_VERSION\s+)\d+$",
            r"\g<1>255",
            header,
        )
        self.assertEqual(255, module["_manual_revision"](manual_revision))
        with self.assertRaisesRegex(ValueError, "between 0 and 255"):
            module["_manual_revision"](
                re.sub(
                    r"(?m)(^#define VSCPD_REVISION_VERSION\s+)\d+$",
                    r"\g<1>256",
                    header,
                )
            )
        updated = module["_update_version_header"](
            manual_revision, datetime(2026, 9, 1), 3925
        )
        self.assertRegex(updated, r"(?m)^#define VSCPD_PATCH_VERSION\s+3925$")
        self.assertRegex(
            updated, r'(?m)^#define VSCPD_DISPLAY_VERSION\s+"26\.09\.255\.3925"$'
        )

    def test_invalid_header_fails_without_writing_metadata(self):
        header = self.header.read_text(encoding="utf-8")
        invalid_headers = (
            re.sub(r"(?m)^#define VSCPD_REVISION_VERSION[^\n]*\n", "", header),
            re.sub(
                r"(?m)^#define VSCPD_REVISION_VERSION[^\n]*\n",
                "#define VSCPD_REVISION_VERSION invalid\n",
                header,
            ),
            re.sub(
                r"(?m)^#define VSCPD_REVISION_VERSION[^\n]*\n",
                "#define VSCPD_REVISION_VERSION 256\n",
                header,
            ),
            re.sub(r"(?m)^#define VSCPD_PATCH_VERSION[^\n]*\n", "", header),
            header + "\n#define VSCPD_PATCH_VERSION 0\n",
            header + "\n#define VSCPD_PATCH_VERSION invalid\n",
        )
        for invalid_header in invalid_headers:
            with self.subTest(header=invalid_header[-40:]):
                self.header.write_text(invalid_header, encoding="utf-8")
                original_variables = self.variables.read_text(encoding="utf-8")
                result = self.update("--prepare-commit")
                self.assertNotEqual(0, result.returncode)
                self.assertTrue(
                    "VSCPD_PATCH_VERSION" in result.stderr
                    or "VSCPD_REVISION_VERSION" in result.stderr,
                    result.stderr,
                )
                self.assertEqual(invalid_header, self.header.read_text(encoding="utf-8"))
                self.assertEqual(original_variables, self.variables.read_text(encoding="utf-8"))

    def test_preparing_commit_requires_git(self):
        original_header = self.header.read_text(encoding="utf-8")
        original_variables = self.variables.read_text(encoding="utf-8")
        env = os.environ.copy()
        env["PATH"] = str(self.root / "no-git")
        result = self.update("--prepare-commit", env=env)
        self.assertNotEqual(0, result.returncode)
        self.assertEqual(original_header, self.header.read_text(encoding="utf-8"))
        self.assertEqual(original_variables, self.variables.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
