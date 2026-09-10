#!/usr/bin/env python3
"""Exercise provider qualification through real CMake configure/build/install runs."""

# Copyright 2026 Qore Technologies, s.r.o.
# SPDX-License-Identifier: MIT

import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile


class JavaProviderProfileBuildTest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="java-provider-build-")
        self.addCleanup(temporary.cleanup)
        self.base = Path(temporary.name)
        self.source = self.base / "source with spaces"
        self.build = self.base / "build"
        self.jar_dir = self.source / "qlib/ExampleDataProvider/jar"
        self.jar_dir.mkdir(parents=True)
        self.generated = self.jar_dir / "qore-dataprovider-example.jar"
        self.dependency = self.jar_dir / "dependency-1.0.jar"
        for path in (self.source / "generated.jar", self.dependency):
            with zipfile.ZipFile(path, "w") as archive:
                archive.writestr("fixture", b"provider dependency")
        checksum = hashlib.sha256(self.dependency.read_bytes()).hexdigest()
        (self.source / "qlib/java-provider-dependencies.sha256").write_text(
            f"{checksum}  qlib/ExampleDataProvider/jar/{self.dependency.name}\n",
            encoding="utf-8",
        )
        self.module = self.jar_dir.parent / "ExampleDataProvider.qm"
        self.module.write_text(
            "%modern\n%module ExampleDataProvider\n"
            "%module-cmd(jni) add-relative-classpath ./jar/dependency-1.0.jar\n"
            "%module-cmd(jni) add-relative-classpath ./jar/qore-dataprovider-example.jar\n",
            encoding="utf-8",
        )
        validator_dir = self.source / "test/docker_test"
        validator_dir.mkdir(parents=True)
        shutil.copyfile(
            self.repo / "test/docker_test/validate-java-provider-profiles.py",
            validator_dir / "validate-java-provider-profiles.py",
        )
        (self.source / "CMakeLists.txt").write_text(
            "cmake_minimum_required(VERSION 3.16)\n"
            "project(JavaProviderProfileBuild NONE)\n"
            "option(QORE_BUILD_AOT_MODULES \"Build provider qmods\" ON)\n"
            "option(OMIT_QMOD_TARGET \"Simulate a missing AOT rule\" OFF)\n"
            "include(\"${HELPER}\")\n"
            "set(_jar \"${CMAKE_SOURCE_DIR}/qlib/ExampleDataProvider/jar/"
            "qore-dataprovider-example.jar\")\n"
            "add_custom_command(OUTPUT \"${_jar}\"\n"
            "    COMMAND \"${CMAKE_COMMAND}\" -E copy\n"
            "        \"${CMAKE_SOURCE_DIR}/generated.jar\" \"${_jar}\"\n"
            "    DEPENDS \"${CMAKE_SOURCE_DIR}/generated.jar\" VERBATIM)\n"
            # Deliberately not ALL: a direct validation build must generate the JAR.
            "add_custom_target(qore-jni DEPENDS \"${_jar}\")\n"
            "if(QORE_BUILD_AOT_MODULES AND NOT OMIT_QMOD_TARGET)\n"
            "    add_custom_target(ExampleDataProvider-qmod\n"
            "        COMMAND \"${CMAKE_COMMAND}\" -E touch\n"
            "            \"${CMAKE_BINARY_DIR}/qmod-built\" VERBATIM)\n"
            "endif()\n"
            "qore_java_provider_profile(ExampleDataProvider none)\n"
            "qore_finalize_java_provider_profiles()\n",
            encoding="utf-8",
        )
        self.succeed("git", "init", "--quiet", "--initial-branch=main", str(self.source))
        self.succeed("git", "-C", str(self.source), "add", "qlib")

    def run_command(self, *command):
        return subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    def succeed(self, *command):
        result = self.run_command(*command)
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertNotIn("Warning", result.stdout)
        return result

    def configure(self, aot, *extra):
        return self.run_command(
            self.cmake, "-S", str(self.source), "-B", str(self.build),
            f"-DHELPER={self.repo / 'cmake/JavaProviderProfiles.cmake'}",
            f"-DQORE_JAVA_PROVIDER_PYTHON={sys.executable}",
            f"-DQORE_BUILD_AOT_MODULES={aot}", *extra,
        )

    def configure_success(self, aot):
        result = self.configure(aot)
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertNotIn("Warning", result.stdout)

    def build_validation(self, default=False):
        target = [] if default else ["--target", "validate-java-provider-profiles"]
        return self.run_command(self.cmake, "--build", str(self.build), "--parallel", "4", *target)

    def assert_validation_succeeded(self, result):
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertNotIn("Warning", result.stdout)
        self.assertIn("Java provider profiles passed: 1 modules, 2 runtime JAR declarations", result.stdout)
        self.assertEqual((self.source / "generated.jar").read_bytes(), self.generated.read_bytes())

    def test_source_build_generates_jars_and_installs_profiles(self):
        self.configure_success("OFF")
        self.assertFalse(self.generated.exists())
        self.assert_validation_succeeded(self.build_validation(default=True))
        self.assertFalse((self.build / "qmod-built").exists())
        prefix = self.base / "install"
        self.succeed(self.cmake, "--install", str(self.build), "--prefix", str(prefix))
        self.assertEqual(
            (self.build / "java-provider-profiles.tsv").read_bytes(),
            (prefix / "share/qore-jni/java-provider-profiles.tsv").read_bytes(),
        )

    def test_direct_validation_build_generates_jars_without_aot(self):
        self.configure_success("OFF")
        self.assertFalse(self.generated.exists())
        self.assert_validation_succeeded(self.build_validation())

    def test_aot_validation_builds_qmods_and_jars(self):
        self.configure_success("ON")
        self.assertFalse(self.generated.exists())
        self.assert_validation_succeeded(self.build_validation())
        self.assertTrue((self.build / "qmod-built").is_file())

    def test_aot_still_requires_qmod_targets(self):
        result = self.configure("ON", "-DOMIT_QMOD_TARGET=ON")
        self.assertNotEqual(0, result.returncode)
        self.assertIn("Java provider ExampleDataProvider has no qmod build target", result.stdout)

    def test_source_build_still_rejects_missing_jars(self):
        with self.module.open("a", encoding="utf-8") as source:
            source.write("%module-cmd(jni) add-relative-classpath ./jar/missing.jar\n")
        self.configure_success("OFF")
        result = self.build_validation()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("missing runtime JAR:", result.stdout)
        self.assertIn("missing.jar", result.stdout)

    def test_source_build_revalidates_changed_dependencies(self):
        self.configure_success("OFF")
        self.assert_validation_succeeded(self.build_validation())
        with zipfile.ZipFile(self.dependency, "w") as archive:
            archive.writestr("fixture", b"changed dependency")
        result = self.build_validation(default=True)
        self.assertNotEqual(0, result.returncode)
        self.assertIn("checksum mismatch", result.stdout)

    def test_reconfigure_between_source_and_aot_builds(self):
        for aot in ("OFF", "ON", "OFF"):
            with self.subTest(aot=aot):
                self.configure_success(aot)
                self.assert_validation_succeeded(self.build_validation())
                marker = self.build / "qmod-built"
                self.assertEqual(aot == "ON", marker.exists())
                if marker.exists():
                    marker.unlink()


if __name__ == "__main__":
    JavaProviderProfileBuildTest.repo = Path(sys.argv[1]).resolve()
    JavaProviderProfileBuildTest.cmake = sys.argv[2]
    unittest.main(argv=[sys.argv[0]])
