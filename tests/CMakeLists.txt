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

    find_package(GTest REQUIRED)

    enable_testing()
    add_executable(behaviortree_cpp_test ${BT_TESTS})
    add_test(NAME btcpp_test COMMAND behaviortree_cpp_test)

    target_link_libraries(behaviortree_cpp_test
        GTest::gtest
        GTest::gtest_main)

endif()

target_link_libraries(behaviortree_cpp_test ${BTCPP_LIBRARY} bt_sample_nodes foonathan::lexy)
target_include_directories(behaviortree_cpp_test PRIVATE include ${PROJECT_SOURCE_DIR}/3rdparty)
target_compile_definitions(behaviortree_cpp_test PRIVATE BT_TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}")
