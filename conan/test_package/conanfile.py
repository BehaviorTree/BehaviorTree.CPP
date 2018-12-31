#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from conans import ConanFile, CMake


class TestPackageConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        assert os.path.isfile(os.path.join(self.deps_cpp_info["BehaviorTree.CPP"].rootpath, "licenses", "LICENSE"))
        bin_path = os.path.join("bin", "test_package")
        self.run(bin_path, run_environment=True)
