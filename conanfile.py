from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout

class BehaviortreeCppConan(ConanFile):
    name = "behaviortree.cpp"
    settings = "os", "arch", "compiler", "build_type"

    default_options = {
        "flatbuffers/*:header_only": True,
    }

    def layout(self):
        cmake_layout(self)

    def build_requirements(self):
       self.test_requires("gtest/1.14.0")

    def requirements(self):
        self.requires("flatbuffers/24.12.23")
        self.requires("minicoro/0.1.3")
        self.requires("minitrace/cci.20230905")
        self.requires("sqlite3/3.40.1")
        self.requires("tinyxml2/10.0.0")
        self.requires("cppzmq/4.11.0")
        self.requires("foonathan-lexy/2022.12.1")

    def generate(self):
        tc = CMakeToolchain(self)

        tc.cache_variables["USE_VENDORED_CPPZMQ"] = False
        tc.cache_variables["USE_VENDORED_FLATBUFFERS"] = False
        tc.cache_variables["USE_VENDORED_LEXY"] = False
        tc.cache_variables["USE_VENDORED_MINICORO"] = False
        tc.cache_variables["USE_VENDORED_MINITRACE"] = False
        tc.cache_variables["USE_VENDORED_TINYXML2"] = False
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()
