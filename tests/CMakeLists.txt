######################################################
# TESTS

include_directories(include)

set(BT_TESTS
  src/action_test_node.cpp
  src/condition_test_node.cpp
  gtest_tree.cpp
  gtest_sequence.cpp
  gtest_parallel.cpp
  gtest_fallback.cpp
  gtest_factory.cpp
  gtest_decorator.cpp
  gtest_blackboard.cpp
  navigation_test.cpp
  gtest_subtree.cpp
)

if (NOT MINGW)
    list(APPEND BT_TESTS  gtest_coroutines.cpp)
endif()

if(ament_cmake_FOUND AND BUILD_TESTING)

    find_package(ament_cmake_gtest REQUIRED)

    ament_add_gtest_executable(${BEHAVIOR_TREE_LIBRARY}_test ${BT_TESTS})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY}_test ${BEHAVIOR_TREE_LIBRARY}
                                                        bt_sample_nodes
                                                        ${ament_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include)
    include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty>)

elseif(catkin_FOUND AND CATKIN_ENABLE_TESTING)

    catkin_add_gtest(${BEHAVIOR_TREE_LIBRARY}_test ${BT_TESTS})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY}_test ${BEHAVIOR_TREE_LIBRARY}
                                                        bt_sample_nodes
                                                        ${catkin_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include)

elseif(GTEST_FOUND AND BUILD_UNIT_TESTS)

    enable_testing()

    add_executable(${BEHAVIOR_TREE_LIBRARY}_test ${BT_TESTS})
    target_link_libraries(${PROJECT_NAME}_test ${BEHAVIOR_TREE_LIBRARY}
                                                bt_sample_nodes
                                                ${GTEST_LIBRARIES}
                                                ${GTEST_MAIN_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include ${GTEST_INCLUDE_DIRS})

    add_test(BehaviorTreeCoreTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BEHAVIOR_TREE_LIBRARY}_test)

endif()
