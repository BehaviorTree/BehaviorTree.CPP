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
    src/_pybt/exceptions.cpp
    src/_pybt/bind_basic_types.cpp
    src/_pybt/bind_ports.cpp
    src/_pybt/bind_tree_node.cpp
    src/_pybt/bind_factory.cpp
    src/_pybt/bind_tree.cpp
)

target_link_libraries(_pybt PRIVATE ${BTCPP_LIBRARY})

target_compile_features(_pybt PRIVATE cxx_std_17)

if(UNIX AND NOT APPLE)
    set_target_properties(_pybt PROPERTIES
        INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(APPLE)
    set_target_properties(_pybt PROPERTIES
        INSTALL_RPATH "@loader_path;@loader_path/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# Install the compiled extension into the pybt package directory so the
# scikit-build-core wheel ships pybt/_pybt*.so alongside pybt/__init__.py.
install(TARGETS _pybt LIBRARY DESTINATION pybt)

if(UNIX AND NOT APPLE)
    # No Python/nanobind symbols may appear in libbehaviortree_cpp.
    # This must NEVER FAIL. Otherwise the architecture rule is fundamentally broken.
    add_test(NAME pybt_no_python_symbols_in_core
        COMMAND sh -c "! nm -D $<TARGET_FILE:${BTCPP_LIBRARY}> | grep -qE 'PyObject|pybind|nanobind|nb::'"
    )

    # Smoke import under the manylinux-equivalent compile flags.
    # The workflow re-runs the whole build with those flags and invokes
    # this test to verify `import pybt` still resolves cleanly.
    add_test(NAME pybt_import_under_hidden_lto
        COMMAND ${Python_EXECUTABLE} -c "import pybt; pybt.BehaviorTreeFactory()"
    )
    set_tests_properties(pybt_import_under_hidden_lto PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/src:$<TARGET_FILE_DIR:_pybt>"
    )
endif()
