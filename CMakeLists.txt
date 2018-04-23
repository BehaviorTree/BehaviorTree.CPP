cmake_minimum_required(VERSION 2.8)
project(behavior_tree_core)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pthread -Werror=return-type")

#############################################################
# http://answers.ros.org/question/230877/optionally-build-a-package-with-catkin/
if( CATKIN_DEVEL_PREFIX OR CATKIN_BUILD_BINARY_PACKAGE)
    set(catkin_FOUND 1)
    add_definitions( -DUSING_ROS )
endif()

if(catkin_FOUND)
  find_package(catkin REQUIRED COMPONENTS )

  message(STATUS "------------------------------------------")
  message(STATUS "BehaviourTree is being built using CATKIN.")
  message(STATUS "------------------------------------------")

  catkin_package(
    INCLUDE_DIRS include
    LIBRARIES behavior_tree_core
    )
else(catkin_FOUND)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	
    find_package(GTest)
    include_directories(${GTEST_INCLUDE_DIRS})
    if(NOT GTEST_FOUND)
        message(WARNING " GTest not found!")
    endif(NOT GTEST_FOUND)

endif(catkin_FOUND)


set(BT_Source
	src/action_node.cpp
        src/decorator_node.cpp
	src/condition_node.cpp
	src/control_node.cpp
	src/exceptions.cpp
	src/leaf_node.cpp
	src/tick_engine.cpp
	src/parallel_node.cpp
	src/fallback_node.cpp
	src/sequence_node.cpp
	src/fallback_node_with_memory.cpp
	src/sequence_node_with_memory.cpp
	src/tree_node.cpp
        src/bt_factory.cpp
)
include_directories(include)


######################################################
# COMPILING LIBRARY
######################################################
add_library(behavior_tree_core STATIC ${BT_Source} )


######################################################
# TESTS
######################################################
set(BT_Tests
        gtest/src/action_test_node.cpp
        gtest/src/condition_test_node.cpp
        gtest/gtest_tree.cpp
)

if(catkin_FOUND AND CATKIN_ENABLE_TESTING)
    include_directories(gtest/include)

    catkin_add_gtest(behavior_tree_core_test ${BT_Tests} )
    target_link_libraries(behavior_tree_core_test
        behavior_tree_core
        ${catkin_LIBRARIES} )
	    
elseif(GTEST_FOUND)
    include_directories(gtest/include)

    add_executable(behavior_tree_core_test ${BT_Tests} )
    target_link_libraries(behavior_tree_core_test
        behavior_tree_core
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES} )
		
endif()

######################################################
# COMPILING EXAMPLES
######################################################

#TODO


######################################################
# INSTALLATION OF LIBRARY AND EXECUTABLE
######################################################


#TODO
