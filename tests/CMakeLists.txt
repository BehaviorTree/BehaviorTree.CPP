######################################################
# TESTS

set(BT_TESTS
  src/action_test_node.cpp
  src/condition_test_node.cpp

  gtest_any.cpp
  gtest_blackboard.cpp
  gtest_coroutines.cpp
  gtest_decorator.cpp
  gtest_enums.cpp
  gtest_factory.cpp
  gtest_fallback.cpp
  gtest_parallel.cpp
  gtest_preconditions.cpp
  gtest_ports.cpp
  gtest_postconditions.cpp
  gtest_match.cpp
  gtest_json.cpp
  gtest_reactive.cpp
  gtest_reactive_backchaining.cpp
  gtest_sequence.cpp
  gtest_skipping.cpp
  gtest_substitution.cpp
  gtest_subtree.cpp
  gtest_switch.cpp
  gtest_tree.cpp
  gtest_updates.cpp
  gtest_wakeup.cpp
  gtest_interface.cpp

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
target_link_libraries(behaviortree_cpp_test ${BTCPP_LIBRARY} bt_sample_nodes foonathan::lexy)
target_compile_definitions(behaviortree_cpp_test PRIVATE BT_TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}")
