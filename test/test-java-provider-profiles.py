#!/usr/bin/env python3
"""Regression tests for Java provider dependency-profile qualification."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile


class JavaProviderProfileTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.base = Path(self.temporary.name)
        self.root = self.base / "qlib"
        self.module = self.root / "ExampleDataProvider"
        self.jar_dir = self.module / "jar"
        self.jar_dir.mkdir(parents=True)
        self.profiles = self.base / "profiles.tsv"
        self.validator = Path(sys.argv[1]).resolve()

    def tearDown(self):
        self.temporary.cleanup()

    def make_jar(self, name, entries=None, marker=b"fixture"):
        path = self.jar_dir / name
        with zipfile.ZipFile(path, "w") as archive:
            archive.writestr("fixture", marker)
            for entry, value in (entries or {}).items():
                archive.writestr(entry, value)
        return path

    def write_profile(self, policy, jars):
        lines = ["# module\tlogging-policy\truntime-jar"]
        lines.extend(
            f"ExampleDataProvider\t{policy}\t./jar/{jar}"
            for jar in jars
        )
        self.profiles.write_text("\n".join(lines) + "\n", encoding="utf-8")

    def run_validator(self, *extra):
        return subprocess.run(
            [sys.executable, str(self.validator), "--profiles", str(self.profiles),
             "--root", str(self.root), *map(str, extra)],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def make_valid_slf4j(self):
        self.make_jar("slf4j-api-2.0.16.jar")
        self.make_jar("slf4j-nop-2.0.16.jar", {
            "META-INF/services/org.slf4j.spi.SLF4JServiceProvider":
                "org.slf4j.nop.NOPServiceProvider\n",
            "org/slf4j/nop/NOPServiceProvider.class": b"fixture",
        })

    def test_valid_slf4j_nop_profile(self):
        self.make_valid_slf4j()
        self.write_profile("slf4j-nop", (
            "slf4j-api-2.0.16.jar", "slf4j-nop-2.0.16.jar"))
        result = self.run_validator()
        self.assertEqual(0, result.returncode, result.stderr)

    def test_version_mismatch_is_rejected(self):
        self.make_jar("slf4j-api-2.0.16.jar")
        self.make_jar("slf4j-nop-2.0.15.jar", {
            "META-INF/services/org.slf4j.spi.SLF4JServiceProvider":
                "org.slf4j.nop.NOPServiceProvider\n",
            "org/slf4j/nop/NOPServiceProvider.class": b"fixture",
        })
        self.write_profile("slf4j-nop", (
            "slf4j-api-2.0.16.jar", "slf4j-nop-2.0.15.jar"))
        result = self.run_validator()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("differ", result.stderr)

    def test_duplicate_provider_is_rejected(self):
        self.make_valid_slf4j()
        self.make_jar("slf4j-simple-2.0.16.jar", {
            "META-INF/services/org.slf4j.spi.SLF4JServiceProvider":
                "org.slf4j.simple.SimpleServiceProvider\n",
            "org/slf4j/simple/SimpleServiceProvider.class": b"fixture",
        })
        self.write_profile("slf4j-nop", (
            "slf4j-api-2.0.16.jar", "slf4j-nop-2.0.16.jar",
            "slf4j-simple-2.0.16.jar"))
        result = self.run_validator()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("expected one logging provider JAR", result.stderr)

    def test_bridge_cycle_is_rejected(self):
        self.make_valid_slf4j()
        self.make_jar("log4j-to-slf4j-2.24.3.jar")
        self.make_jar("log4j-slf4j2-impl-2.24.3.jar")
        self.write_profile("slf4j-nop", (
            "slf4j-api-2.0.16.jar", "slf4j-nop-2.0.16.jar",
            "log4j-to-slf4j-2.24.3.jar", "log4j-slf4j2-impl-2.24.3.jar"))
        result = self.run_validator()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("logging bridge cycle", result.stderr)

    def test_missing_service_provider_class_is_rejected(self):
        self.make_jar("slf4j-api-2.0.16.jar")
        self.make_jar("slf4j-nop-2.0.16.jar", {
            "META-INF/services/org.slf4j.spi.SLF4JServiceProvider":
                "org.slf4j.nop.NOPServiceProvider\n",
        })
        self.write_profile("slf4j-nop", (
            "slf4j-api-2.0.16.jar", "slf4j-nop-2.0.16.jar"))
        result = self.run_validator()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("provider class is missing", result.stderr)

    def test_staged_content_must_match_source(self):
        source = self.make_jar("qore-dataprovider-example.jar", marker=b"source")
        self.write_profile("none", (source.name,))
        staged_root = self.base / "staged"
        staged_jar_dir = staged_root / "ExampleDataProvider" / "jar"
        staged_jar_dir.mkdir(parents=True)
        with zipfile.ZipFile(staged_jar_dir / source.name, "w") as archive:
            archive.writestr("fixture", b"different")
        result = self.run_validator(
            "--root", staged_root, "--reference-root", self.root)
        self.assertNotEqual(0, result.returncode)
        self.assertIn("differs from source", result.stderr)

    def test_structured_report_is_atomic_and_records_success_or_failure(self):
        report = self.base / "qualification" / "profile.json"
        self.make_valid_slf4j()
        self.write_profile("slf4j-nop", (
            "slf4j-api-2.0.16.jar", "slf4j-nop-2.0.16.jar"))
        result = self.run_validator("--scope", "source", "--report", report)
        self.assertEqual(0, result.returncode, result.stderr)
        payload = json.loads(report.read_text(encoding="utf-8"))
        self.assertTrue(payload["complete"])
        self.assertEqual("source", payload["qualification"]["scope"])
        self.assertEqual(1, payload["qualification"]["module_count"])
        self.assertEqual(2, payload["qualification"]["runtime_jar_declarations"])
        self.assertEqual([], payload["errors"])

        self.write_profile("none", ("slf4j-api-2.0.16.jar",))
        result = self.run_validator("--scope", "source", "--report", report)
        self.assertNotEqual(0, result.returncode)
        payload = json.loads(report.read_text(encoding="utf-8"))
        self.assertFalse(payload["complete"])
        self.assertTrue(payload["errors"])
        self.assertEqual([], list(report.parent.glob(".*.tmp.*")))


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]])
