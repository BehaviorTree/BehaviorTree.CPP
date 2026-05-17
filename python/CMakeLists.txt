cmake_minimum_required(VERSION 3.18)

# pybt — Python bindings for BehaviorTree.CPP.
#
# Every pybind/nanobind/Python symbol must live in _pybt only. 
# The behaviortree_cpp target must remain Python-unaware. 
# Linkage is one-way: _pybt -> behaviortree_cpp.

find_package(Python 3.9
    COMPONENTS Interpreter Development.Module
    REQUIRED)

find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_pybt
    NB_STATIC
    STABLE_ABI
    src/_pybt/module.cpp
)

target_link_libraries(_pybt PRIVATE ${BTCPP_LIBRARY})

target_compile_features(_pybt PRIVATE cxx_std_17)

# Install the compiled extension into the pybt package directory so the
# scikit-build-core wheel ships pybt/_pybt*.so alongside pybt/__init__.py.
install(TARGETS _pybt LIBRARY DESTINATION pybt)
