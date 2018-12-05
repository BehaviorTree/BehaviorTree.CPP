#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Conan recipe package for BehaviorTree.CPP
"""
from conans import ConanFile, CMake, tools


class BehaviorTreeConan(ConanFile):
    name = "BehaviorTree.CPP"
    license = "MIT"
    url = "https://github.com/BehaviorTree/BehaviorTree.CPP"
    author = "Davide Faconti <davide.faconti@gmail.com>"
    topics = ("conan", "behaviortree", "ai", "robotics", "games", "coordination")
    description = "This C++ library provides a framework to create BehaviorTrees. It was designed to be flexible, easy to use and fast."
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    exports = "LICENSE"
    exports_sources = ("cmake/*", "include/*", "src/*", "3rdparty/*", "CMakeLists.txt")

    def requirements(self):

        self.requires("cppzmq/4.3.0@bincrafters/stable")

    def _configure_cmake(self):
        """Create CMake instance and execute configure step
        """
        cmake = CMake(self)
        cmake.definitions["BUILD_EXAMPLES"] = False
        cmake.definitions["BUILD_UNIT_TESTS"] = False
        cmake.configure()
        return cmake

    def build(self):
        """Configure, build and install BehaviorTree using CMake.
        """
        tools.replace_in_file("CMakeLists.txt",
                              "project(behaviortree_cpp)",
                              """project(behaviortree_cpp)
                              include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
                              conan_basic_setup()""")
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        """Copy BehaviorTree artifacts to package folder
        """
        self.copy(pattern="LICENSE", dst="licenses")
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        """Collect built libraries names and solve pthread path.
        """
        self.cpp_info.libs = tools.collect_libs(self)
        if self.settings.os == "Linux":
            self.cpp_info.libs.append("pthread")
