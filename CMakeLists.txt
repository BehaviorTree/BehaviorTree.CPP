cmake_minimum_required(VERSION 2.8.12) # version on Ubuntu Trusty
project(behaviortree_cpp)

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(CMAKE_CONFIG_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

option(BUILD_EXAMPLES   "Build tutorials and examples" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)

#############################################################
# Find packages
find_package(Threads REQUIRED)
find_package(ZMQ)

list(APPEND BEHAVIOR_TREE_EXTERNAL_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

if( ZMQ_FOUND )
    message(STATUS "ZeroMQ found.")
    add_definitions( -DZMQ_FOUND )
    list(APPEND BT_SOURCE src/loggers/bt_zmq_publisher.cpp)
    list(APPEND BEHAVIOR_TREE_EXTERNAL_LIBRARIES zmq)
else()
    message(WARNING "ZeroMQ NOT found. Skipping the build of [PublisherZMQ] and [bt_recorder].")
endif()

set(BEHAVIOR_TREE_LIBRARY ${PROJECT_NAME})

# Update the policy setting to avoid an error when loading the ament_cmake package
# at the current cmake version level
if(POLICY CMP0057)
   cmake_policy(SET CMP0057 NEW)
endif()

find_package(ament_cmake QUIET)

if ( ament_cmake_FOUND )
  find_package(ament_cmake_gtest REQUIRED)

  # Not adding -DUSING_ROS since xml_parsing.cpp hasn't been ported to ROS2

  message(STATUS "------------------------------------------")
  message(STATUS "BehaviourTree is being built using AMENT.")
  message(STATUS "------------------------------------------")

  set(BUILD_TOOL_INCLUDE_DIRS ${ament_INCLUDE_DIRS})

elseif( CATKIN_DEVEL_PREFIX OR CATKIN_BUILD_BINARY_PACKAGE)

  set(catkin_FOUND 1)
  add_definitions( -DUSING_ROS )
  find_package(catkin REQUIRED COMPONENTS roslib)
  find_package(GTest)

  message(STATUS "------------------------------------------")
  message(STATUS "BehaviourTree is being built using CATKIN.")
  message(STATUS "------------------------------------------")

  catkin_package(
    INCLUDE_DIRS include # do not include "3rdparty" here
    LIBRARIES ${BEHAVIOR_TREE_LIBRARY}
    CATKIN_DEPENDS roslib
    )

  list(APPEND BEHAVIOR_TREE_EXTERNAL_LIBRARIES ${catkin_LIBRARIES})
  set(BUILD_TOOL_INCLUDE_DIRS ${catkin_INCLUDE_DIRS})

else()
    find_package(GTest)

    if(NOT GTEST_FOUND)
      message(WARNING " GTest not found!")
    endif(NOT GTEST_FOUND)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

endif()

#############################################################
# LIBRARY

list(APPEND BT_SOURCE
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
    src/decorators/subtree_node.cpp

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



add_library(${BEHAVIOR_TREE_LIBRARY} SHARED ${BT_SOURCE} )
target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PUBLIC
    ${BEHAVIOR_TREE_EXTERNAL_LIBRARIES})

target_include_directories(${BEHAVIOR_TREE_LIBRARY} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty>
    $<INSTALL_INTERFACE:3rdparty>
    ${BUILD_TOOL_INCLUDE_DIRS})

if(MSVC)
  target_compile_options(${BEHAVIOR_TREE_LIBRARY} PRIVATE /W4 /WX)
else()
  target_compile_options(${BEHAVIOR_TREE_LIBRARY} PRIVATE -Wall -Wextra -Werror=return-type)
endif()

######################################################
# EXPORTS

set(PROJECT_NAMESPACE BehaviorTree)
set(PROJECT_CONFIG ${PROJECT_NAMESPACE}Config)
export(TARGETS ${PROJECT_NAME}
       NAMESPACE ${PROJECT_NAMESPACE}::
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_CONFIG}.cmake")

######################################################
# TESTS

set(BT_TESTS
  gtest/src/action_test_node.cpp
  gtest/src/condition_test_node.cpp
  gtest/gtest_tree.cpp
  gtest/gtest_sequence.cpp
  gtest/gtest_parallel.cpp
  gtest/gtest_fallback.cpp
  gtest/gtest_factory.cpp
  gtest/gtest_decorator.cpp
  gtest/gtest_blackboard.cpp
  gtest/navigation_test.cpp
)

if(ament_cmake_FOUND AND BUILD_TESTING)

    find_package(ament_cmake_gtest REQUIRED)

    ament_add_gtest_executable(${BEHAVIOR_TREE_LIBRARY}_test ${BT_TESTS})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY}_test ${BEHAVIOR_TREE_LIBRARY}
                                                        crossdoor_nodes
                                                        ${ament_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include)
    include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty>)

elseif(catkin_FOUND AND CATKIN_ENABLE_TESTING)

    catkin_add_gtest(${BEHAVIOR_TREE_LIBRARY}_test ${BT_TESTS})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY}_test ${BEHAVIOR_TREE_LIBRARY}
                                                        crossdoor_nodes
                                                        ${catkin_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include)

elseif(GTEST_FOUND AND BUILD_UNIT_TESTS)

    enable_testing()

    add_executable(${BEHAVIOR_TREE_LIBRARY}_test ${BT_TESTS})
    target_link_libraries(${PROJECT_NAME}_test ${BEHAVIOR_TREE_LIBRARY}
                                                crossdoor_nodes
                                                ${GTEST_LIBRARIES}
                                                ${GTEST_MAIN_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include ${GTEST_INCLUDE_DIRS})

    add_test(BehaviorTreeCoreTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BEHAVIOR_TREE_LIBRARY}_test)
endif()

######################################################
# INSTALL
if(ament_cmake_FOUND)
    set( BEHAVIOR_TREE_LIB_DESTINATION   lib )
    set( BEHAVIOR_TREE_INC_DESTINATION   include )
    set( BEHAVIOR_TREE_BIN_DESTINATION   bin )

    ament_export_include_directories(include)
    ament_export_libraries(${BEHAVIOR_TREE_LIBRARY})
    ament_package()
elseif(catkin_FOUND)
    set( BEHAVIOR_TREE_LIB_DESTINATION   ${CATKIN_PACKAGE_LIB_DESTINATION} )
    set( BEHAVIOR_TREE_INC_DESTINATION   ${CATKIN_PACKAGE_INCLUDE_DESTINATION} )
    set( BEHAVIOR_TREE_BIN_DESTINATION   ${CATKIN_PACKAGE_BIN_DESTINATION} )
else()
    set( BEHAVIOR_TREE_LIB_DESTINATION   lib )
    set( BEHAVIOR_TREE_INC_DESTINATION   include )
    set( BEHAVIOR_TREE_BIN_DESTINATION   bin )
endif()

message(STATUS "BEHAVIOR_TREE_LIB_DESTINATION: ${BEHAVIOR_TREE_LIB_DESTINATION}")
message(STATUS "BEHAVIOR_TREE_INC_DESTINATION: ${BEHAVIOR_TREE_INC_DESTINATION}")
message(STATUS "BEHAVIOR_TREE_BIN_DESTINATION: ${BEHAVIOR_TREE_BIN_DESTINATION}")

INSTALL(TARGETS ${BEHAVIOR_TREE_LIBRARY}
    EXPORT ${PROJECT_CONFIG}
    ARCHIVE DESTINATION ${BEHAVIOR_TREE_LIB_DESTINATION}
    LIBRARY DESTINATION ${BEHAVIOR_TREE_LIB_DESTINATION}
    )

INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${BEHAVIOR_TREE_INC_DESTINATION}
    FILES_MATCHING PATTERN "*.h*")

install(EXPORT ${PROJECT_CONFIG}
    DESTINATION "${BEHAVIOR_TREE_LIB_DESTINATION}/${PROJECT_NAMESPACE}/cmake"
    NAMESPACE ${PROJECT_NAMESPACE}::)

######################################################
# EXAMPLES and TOOLS

if( BUILD_EXAMPLES )
    add_subdirectory(tools)
    add_subdirectory(sample_nodes)
    add_subdirectory(examples)
endif()


