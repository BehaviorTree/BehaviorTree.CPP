cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(behavior_tree_core VERSION 1.0.0)

option(BUILD_EXAMPLES   "Build tutorials and examples" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)

if( CATKIN_DEVEL_PREFIX OR CATKIN_BUILD_BINARY_PACKAGE)
    set(catkin_FOUND 1)
    add_definitions( -DUSING_ROS )
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -Wall -Wpedantic -Wattributes")

set(CMAKE_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD_FLAG TRUE)

set(BEHAVIOR_TREE_LIBRARY ${PROJECT_NAME})

find_package(Threads REQUIRED)
find_package(ZMQ)
find_package(GTest)

list(APPEND BEHAVIOR_TREE_EXTERNAL_LIBRARIES Threads::Threads ${CMAKE_DL_LIBS})

if( ZMQ_FOUND )
    message(STATUS "ZeroMQ found.")
    add_definitions( -DZMQ_FOUND )
    list(APPEND BT_Source src/loggers/bt_zmq_publisher.cpp)
    list(APPEND BEHAVIOR_TREE_EXTERNAL_LIBRARIES zmq)
else()
    message(WARNING "ZeroMQ NOT found. Skipping the build of [PublisherZMQ] and [bt_recorder].")
endif()

######################################################
# SOURCES
######################################################
list(APPEND BT_Source
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

######################################################
# TESTS
######################################################
list(APPEND BT_Tests
        gtest/src/action_test_node.cpp
        gtest/src/condition_test_node.cpp
        gtest/gtest_tree.cpp
        gtest/gtest_sequence.cpp
        gtest/gtest_parallel.cpp
        gtest/gtest_fallback.cpp
        gtest/gtest_factory.cpp
        gtest/gtest_decorator.cpp
)

######################################################
# LIBRARY
######################################################
if(catkin_FOUND)
  find_package(catkin REQUIRED COMPONENTS)

  message(STATUS "------------------------------------------")
  message(STATUS "BehaviourTree is being built using CATKIN.")
  message(STATUS "------------------------------------------")

  catkin_package(
    INCLUDE_DIRS
        include 
        3rdparty
    LIBRARIES 
        ${BEHAVIOR_TREE_LIBRARY}
    )

    add_library(${BEHAVIOR_TREE_LIBRARY} STATIC ${BT_Source})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE ${BEHAVIOR_TREE_EXTERNAL_LIBRARIES} ${catkin_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY} PUBLIC include 3rdparty ${catkin_INCLUDE_DIRS})

else()
    include(GNUInstallDirs)

    set(LIB_INSTALL_PATH          ${CMAKE_INSTALL_LIBDIR})
    set(CONFIG_FILES_INSTALL_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/${BEHAVIOR_TREE_LIBRARY}-${${PROJECT_NAME}_VERSION})
    set(INCLUDE_INSTALL_PATH      ${CMAKE_INSTALL_INCLUDEDIR}/${BEHAVIOR_TREE_LIBRARY}-${${PROJECT_NAME}_VERSION})

    add_library(${BEHAVIOR_TREE_LIBRARY} STATIC ${BT_Source})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE ${BEHAVIOR_TREE_EXTERNAL_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}
                                PUBLIC
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_PATH}>
                                PUBLIC
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty>
                                    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_PATH}>
                               )

    set_target_properties(${BEHAVIOR_TREE_LIBRARY} PROPERTIES PREFIX ""
                                                              SUFFIX ".a.${${PROJECT_NAME}_VERSION}")
endif()

######################################################
# EXAMPLES and TOOLS
######################################################
if( BUILD_EXAMPLES )
    add_subdirectory(tools)
    add_subdirectory(sample_nodes)
    add_subdirectory(examples)
endif()

######################################################
# TESTS
######################################################
if(catkin_FOUND AND CATKIN_ENABLE_TESTING)
    catkin_add_gtest(${BEHAVIOR_TREE_LIBRARY}_test ${BT_Tests})
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY}_test ${BEHAVIOR_TREE_LIBRARY}
                                                        crossdoor_nodes
                                                        ${catkin_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include)
elseif(GTEST_FOUND AND BUILD_UNIT_TESTS)
    enable_testing()

    add_executable(${BEHAVIOR_TREE_LIBRARY}_test ${BT_Tests})
    target_link_libraries(${PROJECT_NAME}_test ${BEHAVIOR_TREE_LIBRARY}
                                                crossdoor_nodes
                                                ${GTEST_LIBRARIES}
                                                ${GTEST_MAIN_LIBRARIES})
    target_include_directories(${BEHAVIOR_TREE_LIBRARY}_test PRIVATE gtest/include ${GTEST_INCLUDE_DIRS})

    add_test(BehaviorTreeCoreTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BEHAVIOR_TREE_LIBRARY}_test)
elseif(NOT GTEST_FOUND)
    message(WARNING " GTest not found!")
endif()

######################################################
# INSTALL
######################################################
if(catkin_FOUND)
    install(TARGETS ${BEHAVIOR_TREE_LIBRARY}
            ARCHIVE  DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
            )
else()
    install(TARGETS ${BEHAVIOR_TREE_LIBRARY}
            EXPORT  ${BEHAVIOR_TREE_LIBRARY}-targets
            ARCHIVE  DESTINATION ${LIB_INSTALL_PATH}
            )

    install(EXPORT ${BEHAVIOR_TREE_LIBRARY}-targets
            FILE ${BEHAVIOR_TREE_LIBRARY}-targets.cmake
            DESTINATION ${CONFIG_FILES_INSTALL_PATH}
            )

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${BEHAVIOR_TREE_LIBRARY}-config-version.cmake"
        VERSION ${${PROJECT_NAME}_VERSION}
        COMPATIBILITY SameMajorVersion
        )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BEHAVIOR_TREE_LIBRARY}-config.cmake
                  ${CMAKE_CURRENT_BINARY_DIR}/${BEHAVIOR_TREE_LIBRARY}-config-version.cmake
            DESTINATION ${CONFIG_FILES_INSTALL_PATH}
            )

    install(DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/include/
                        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/
            DESTINATION ${INCLUDE_INSTALL_PATH}
            )

    configure_file(${CMAKE_CONFIG_PATH}/${BEHAVIOR_TREE_LIBRARY}-config.cmake.in
                   ${CMAKE_CURRENT_BINARY_DIR}/${BEHAVIOR_TREE_LIBRARY}-config.cmake
                   @ONLY
                   )
endif()
