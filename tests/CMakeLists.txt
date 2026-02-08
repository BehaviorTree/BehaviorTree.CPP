######################################################
# TESTS

######################################################
# Plugin for Issue #953 test (must be built before tests)
# This plugin has a custom type with convertFromString ONLY in the plugin
add_library(plugin_issue953 SHARED plugin_issue953/plugin_issue953.cpp)
target_compile_definitions(plugin_issue953 PRIVATE BT_PLUGIN_EXPORT)
set_target_properties(plugin_issue953 PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(plugin_issue953 ${BTCPP_LIBRARY})

######################################################

set(BT_TESTS
  src/action_test_node.cpp
  src/condition_test_node.cpp

  gtest_any.cpp
  gtest_basic_types.cpp
  gtest_blackboard.cpp
  gtest_coroutines.cpp
  gtest_decorator.cpp
  gtest_enums.cpp
  gtest_factory.cpp
  gtest_fallback.cpp
  # gtest_groot2_publisher.cpp  # Disabled due to pre-existing heap corruption issues on Windows/Pixi
  gtest_if_then_else.cpp
  gtest_parallel.cpp
  gtest_preconditions.cpp
  gtest_ports.cpp
  gtest_port_type_rules.cpp
  gtest_postconditions.cpp
  gtest_match.cpp
  gtest_name_validation.cpp
  gtest_json.cpp
  gtest_loggers.cpp
  gtest_loop.cpp
  gtest_reactive.cpp
  gtest_reactive_backchaining.cpp
  gtest_sequence.cpp
  gtest_skipping.cpp
  gtest_substitution.cpp
  gtest_subtree.cpp
  gtest_switch.cpp
  gtest_tree.cpp
  gtest_try_catch.cpp
  gtest_exception_tracking.cpp
  gtest_updates.cpp
  gtest_wakeup.cpp
  gtest_while_do_else.cpp
  gtest_interface.cpp
  gtest_simple_string.cpp
  gtest_polymorphic_ports.cpp
  gtest_plugin_issue953.cpp
  gtest_blackboard_thread_safety.cpp
  gtest_xml_null_subtree_id.cpp

  script_parser_test.cpp
  test_helper.hpp
)

if(ament_cmake_FOUND)

    find_package(ament_cmake_gtest REQUIRED)

    ament_add_gtest(behaviortree_cpp_test ${BT_TESTS})
    target_link_libraries(behaviortree_cpp_test ${ament_LIBRARIES})

else()

    enable_testing()

    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(behaviortree_cpp_test ${BT_TESTS})

    target_link_libraries(behaviortree_cpp_test
        GTest::gtest
        GTest::gtest_main)

    # gtest_discover_tests queries the test executable for available tests and registers them on ctest individually
    # On Windows it needs a little help to find the shared libraries
    if(WIN32)
        gtest_discover_tests(behaviortree_cpp_test
            DISCOVERY_MODE PRE_TEST
            DISCOVERY_ENVIRONMENT "PATH=$<TARGET_FILE_DIR:behaviortree_cpp_test>;$ENV{PATH}"
        )
    else()
        gtest_discover_tests(behaviortree_cpp_test)
    endif()

endif()

target_include_directories(behaviortree_cpp_test PRIVATE include)
target_link_libraries(behaviortree_cpp_test ${BTCPP_LIBRARY} bt_sample_nodes)
target_compile_definitions(behaviortree_cpp_test PRIVATE BT_TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}")

# Ensure plugin is built before tests run, and tests can find it
add_dependencies(behaviortree_cpp_test plugin_issue953)
target_compile_definitions(behaviortree_cpp_test PRIVATE
  BT_PLUGIN_ISSUE953_PATH="$<TARGET_FILE:plugin_issue953>"
)
