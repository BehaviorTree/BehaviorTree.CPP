cmake_minimum_required(VERSION 2.8)
project(behavior_tree_core)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pthread -Werror=return-type")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
#############################################################
# http://answers.ros.org/question/230877/optionally-build-a-package-with-catkin/
#
# This variable MUST NOT be set manually by the user, it will be detected
# automatically if you compile using catkin
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
    INCLUDE_DIRS include # do not include "3rdparty" here
    LIBRARIES behavior_tree_core
    )

else(catkin_FOUND)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	
    find_package(GTest)
    include_directories(${GTEST_INCLUDE_DIRS})
    if(NOT GTEST_FOUND)
        message(WARNING " GTest not found!")
    endif(NOT GTEST_FOUND)

endif(catkin_FOUND)


set(BT_Source
	src/action_node.cpp
        src/basic_types.cpp
        src/decorator_node.cpp
	src/condition_node.cpp
	src/control_node.cpp
	src/exceptions.cpp
	src/leaf_node.cpp
	src/tick_engine.cpp
        src/tree_node.cpp
        src/bt_factory.cpp
        src/behavior_tree.cpp
        src/xml_parsing.cpp
        src/shared_library.cpp
        src/shared_library_UNIX.cpp

        src/decorators/inverter_node.cpp
        src/decorators/repeat_node.cpp
        src/decorators/retry_node.cpp
        src/decorators/timeout_node.cpp

        src/controls/parallel_node.cpp
        src/controls/sequence_node.cpp
        src/controls/sequence_star_node.cpp
        src/controls/fallback_node.cpp
        src/controls/fallback_star_node.cpp

        src/loggers/bt_cout_logger.cpp
        src/loggers/bt_file_logger.cpp
        src/loggers/bt_minitrace_logger.cpp

        3rdparty/tinyXML2/tinyxml2.cpp
        3rdparty/minitrace/minitrace.cpp
)
include_directories(include 3rdparty/)

set(BEHAVIOR_TREE_LIBRARIES behavior_tree_core)

find_package(ZMQ)

if( ZMQ_FOUND )
    message(STATUS "ZeroMQ found.")
    add_definitions( -DZMQ_FOUND )
    set(BT_Source ${BT_Source} src/loggers/bt_zmq_publisher.cpp )

    set(BEHAVIOR_TREE_LIBRARIES ${BEHAVIOR_TREE_LIBRARIES} zmq)
else()
    message(WARNING "ZeroMQ NOT found. Skipping the build of [PublisherZMQ] and [bt_recorder].")
endif()

######################################################
# LIBRARY
######################################################
add_library(behavior_tree_core STATIC ${BT_Source} )
target_compile_options(behavior_tree_core PRIVATE "-fPIC")
target_link_libraries(behavior_tree_core dl)

######################################################
# EXAMPLES and TOOLS
######################################################

add_subdirectory(tools)
add_subdirectory(sample_nodes)
add_subdirectory(examples)

######################################################
# TESTS
######################################################
set(BT_Tests
        gtest/src/action_test_node.cpp
        gtest/src/condition_test_node.cpp
        gtest/gtest_tree.cpp
        gtest/gtest_sequence.cpp
        gtest/gtest_parallel.cpp
        gtest/gtest_fallback.cpp
        gtest/gtest_factory.cpp
        gtest/gtest_decorator.cpp
)

if(catkin_FOUND AND CATKIN_ENABLE_TESTING)
    include_directories(gtest/include)

    catkin_add_gtest(behavior_tree_core_test ${BT_Tests} )
    target_link_libraries(behavior_tree_core_test
        ${BEHAVIOR_TREE_LIBRARIES} crossdoor_nodes
        ${catkin_LIBRARIES} )
	    
elseif(GTEST_FOUND)
    include_directories(gtest/include)

    add_executable(behavior_tree_core_test ${BT_Tests} )
    target_link_libraries(behavior_tree_core_test
        ${BEHAVIOR_TREE_LIBRARIES} crossdoor_nodes
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        )
		
endif()




