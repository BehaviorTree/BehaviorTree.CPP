from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps

class BehaviortreeCppConan(ConanFile):
    name = "behaviortree.cpp"
    settings = "os", "arch", "compiler", "build_type"

    default_options = {
        "flatbuffers/*:header_only": True,
    }

    def build_requirements(self):
       self.test_requires("gtest/1.14.0")

    def requirements(self):
        self.requires("flatbuffers/24.12.23")
        self.requires("minicoro/0.1.3")
        self.requires("minitrace/cci.20230905")
        self.requires("sqlite3/3.40.1") # This should be a transitive dependency of cpp-sqlite
        self.requires("tinyxml2/10.0.0")
        self.requires("zeromq/4.3.4")

    def generate(self):
        tc = CMakeToolchain(self)
      
        #tc.variables["USE_VENDORED_CPPSQLITE"] = False
        #tc.variables["USE_VENDORED_CPPZMQ"] = False
        tc.variables["USE_VENDORED_FLATBUFFERS"] = False
        #tc.variables["USE_VENDORED_LEXY"] = False
        tc.variables["USE_VENDORED_MINICORO"] = False
        tc.variables["USE_VENDORED_MINITRACE"] = False
        tc.variables["USE_VENDORED_TINYXML2"] = False
        #tc.variables["USE_VENDORED_WILDCARDS"] = False
        tc.generate()
        
        deps = CMakeDeps(self)
        deps.generate()
