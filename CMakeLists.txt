cmake_minimum_required(VERSION 3.10.2) # version on Ubuntu Bionic
project(behaviortree_cpp_v3)

#---- Add the subdirectory cmake ----
set(CMAKE_CONFIG_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

#---- Enable C++14 ----
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
else()
    add_definitions(-Wpedantic)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#---- project configuration ----
option(BUILD_EXAMPLES   "Build tutorials and examples" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)
option(BUILD_TOOLS "Build commandline tools" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_MANUAL_SELECTOR "Build manual selector node" ON)
option(ENABLE_COROUTINES "Enable boost coroutines" ON)

#---- Include boost to add coroutines ----
if(ENABLE_COROUTINES)
    find_package(Boost COMPONENTS coroutine QUIET)

    if(Boost_FOUND)
        string(REPLACE "." "0" Boost_VERSION_NODOT ${Boost_VERSION})
        if(NOT Boost_VERSION_NODOT VERSION_LESS 105900)
            message(STATUS "Found boost::coroutine2.")
            add_definitions(-DBT_BOOST_COROUTINE2)
            set(BT_COROUTINES_FOUND true)
        elseif(NOT Boost_VERSION_NODOT VERSION_LESS 105300)
            message(STATUS "Found boost::coroutine.")
            add_definitions(-DBT_BOOST_COROUTINE)
            set(BT_COROUTINES_FOUND true)
        endif()
    endif()

    if(NOT DEFINED BT_COROUTINES_FOUND)
        message(STATUS "Boost coroutines disabled. Install Boost (version 1.59+ recommended).")
    endif()
else()
    message(STATUS "Boost coroutines disabled by CMake option.")
endif()

if(NOT DEFINED BT_COROUTINES_FOUND)
    add_definitions(-DBT_NO_COROUTINES)
endif()

#---- Find other packages ----
find_package(Threads)
find_package(ZMQ)

if( ZMQ_FOUND )
    message(STATUS "ZeroMQ: found.")
    add_definitions( -DZMQ_FOUND )
    list(APPEND BT_SOURCE src/loggers/bt_zmq_publisher.cpp)
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

    add_definitions( -DUSING_ROS2 )
    message(STATUS "------------------------------------------")
    message(STATUS "BehaviourTree is being built using AMENT.")
    message(STATUS "------------------------------------------")

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

elseif(BUILD_UNIT_TESTS)
    if(${CMAKE_VERSION} VERSION_LESS "3.11.0")
        find_package(GTest REQUIRED)
    else()
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
            )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()


#############################################################
# LIBRARY

list(APPEND BT_SOURCE
    src/action_node.cpp
    src/basic_types.cpp
    src/behavior_tree.cpp
    src/blackboard.cpp
    src/bt_factory.cpp
    src/decorator_node.cpp
    src/condition_node.cpp
    src/control_node.cpp
    src/shared_library.cpp
    src/tree_node.cpp
    src/xml_parsing.cpp

    src/decorators/inverter_node.cpp
    src/decorators/repeat_node.cpp
    src/decorators/retry_node.cpp
    src/decorators/subtree_node.cpp
    src/decorators/delay_node.cpp

    src/controls/if_then_else_node.cpp
    src/controls/fallback_node.cpp
    src/controls/parallel_node.cpp
    src/controls/reactive_sequence.cpp
    src/controls/reactive_fallback.cpp
    src/controls/sequence_node.cpp
    src/controls/sequence_star_node.cpp
    src/controls/switch_node.cpp
    src/controls/while_do_else_node.cpp

    src/loggers/bt_cout_logger.cpp
    src/loggers/bt_file_logger.cpp
    src/loggers/bt_minitrace_logger.cpp

    3rdparty/tinyxml2/tinyxml2.cpp
    3rdparty/minitrace/minitrace.cpp
    )

if(BUILD_MANUAL_SELECTOR)
    find_package(Curses QUIET)
    if(CURSES_FOUND)
        message(STATUS "NCurses: found.")
        list(APPEND BT_SOURCE src/controls/manual_node.cpp )
    else()
        message(WARNING "NCurses NOT found. Skipping the build of manual selector node.")
    endif()
endif()


######################################################

if (UNIX)
    list(APPEND BT_SOURCE src/shared_library_UNIX.cpp )
endif()

if (WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
    list(APPEND BT_SOURCE src/shared_library_WIN.cpp )
endif()

if (BUILD_SHARED_LIBS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_library(${BEHAVIOR_TREE_LIBRARY} SHARED ${BT_SOURCE})
else()
    add_library(${BEHAVIOR_TREE_LIBRARY} STATIC ${BT_SOURCE})
endif()

target_link_libraries(${BEHAVIOR_TREE_LIBRARY}
    PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
)

target_include_directories(${BEHAVIOR_TREE_LIBRARY}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lexy/include>
        $<BUILD_INTERFACE:${BTCPP_EXTRA_INCLUDE_DIRS}>
    )

if(ZMQ_FOUND)
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${ZMQ_LIBRARIES} )
    target_include_directories(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${ZMQ_INCLUDE_DIRS} )
    target_compile_definitions(${BEHAVIOR_TREE_LIBRARY} PRIVATE ZMQ_FOUND)
endif()

if(BT_COROUTINES_FOUND)
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${Boost_LIBRARIES} )
    target_include_directories(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${Boost_INCLUDE_DIRS} )
endif()

if(CURSES_FOUND)
    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${CURSES_LIBRARIES} )
    target_include_directories(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${CURSES_INCLUDE_DIRS} )
    target_compile_definitions(${BEHAVIOR_TREE_LIBRARY} PRIVATE NCURSES_FOUND)
endif()


target_compile_definitions(${BEHAVIOR_TREE_LIBRARY} PRIVATE $<$<CONFIG:Debug>:TINYXML2_DEBUG>)

if(MSVC)
else()
    target_compile_options(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        -Wall -Wextra -Werror=return-type)
endif()

#############################################################
if(ament_cmake_FOUND)
    find_package(ament_index_cpp REQUIRED)

    target_include_directories(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        $<BUILD_INTERFACE:${ament_index_cpp_INCLUDE_DIRS}> )

    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        $<BUILD_INTERFACE:ament_index_cpp::ament_index_cpp> )

    ament_export_dependencies(ament_index_cpp)

    set( BEHAVIOR_TREE_LIB_DESTINATION   lib )
    set( BEHAVIOR_TREE_INC_DESTINATION   include )
    set( BEHAVIOR_TREE_BIN_DESTINATION   bin )

    ament_export_include_directories(include)
    ament_export_libraries(${BEHAVIOR_TREE_LIBRARY})
    ament_package()
elseif(catkin_FOUND)

    target_include_directories(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${catkin_INCLUDE_DIRS} )

    target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PRIVATE
        ${catkin_LIBRARIES} )

    set( BEHAVIOR_TREE_LIB_DESTINATION   ${CATKIN_PACKAGE_LIB_DESTINATION} )
    set( BEHAVIOR_TREE_INC_DESTINATION   ${CATKIN_GLOBAL_INCLUDE_DESTINATION} )
    set( BEHAVIOR_TREE_BIN_DESTINATION   ${CATKIN_GLOBAL_BIN_DESTINATION} )

else()
    set( BEHAVIOR_TREE_LIB_DESTINATION   lib )
    set( BEHAVIOR_TREE_INC_DESTINATION   include )
    set( BEHAVIOR_TREE_BIN_DESTINATION   bin )

endif()

message( STATUS "BEHAVIOR_TREE_LIB_DESTINATION:   ${BEHAVIOR_TREE_LIB_DESTINATION} " )
message( STATUS "BEHAVIOR_TREE_BIN_DESTINATION:   ${BEHAVIOR_TREE_BIN_DESTINATION} " )
message( STATUS "BUILD_UNIT_TESTS:   ${BUILD_UNIT_TESTS} " )

add_subdirectory(sample_nodes)

######################################################
# Test
if (BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()

######################################################
# INSTALL

INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${BEHAVIOR_TREE_INC_DESTINATION}
    FILES_MATCHING PATTERN "*.h*")

if(catkin_FOUND)
    INSTALL(TARGETS ${BEHAVIOR_TREE_LIBRARY}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
    )
else()
    INSTALL(TARGETS ${BEHAVIOR_TREE_LIBRARY}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${BEHAVIOR_TREE_LIB_DESTINATION}
        LIBRARY DESTINATION ${BEHAVIOR_TREE_LIB_DESTINATION}
        RUNTIME DESTINATION ${BEHAVIOR_TREE_BIN_DESTINATION}
    )

    install(EXPORT ${PROJECT_NAME}Targets
        FILE "${PROJECT_NAME}Targets.cmake"
        DESTINATION "${BEHAVIOR_TREE_LIB_DESTINATION}/cmake/${PROJECT_NAME}"
        NAMESPACE BT::
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${BEHAVIOR_TREE_LIB_DESTINATION}/cmake/${PROJECT_NAME}"
    )

    export(PACKAGE ${PROJECT_NAME})

    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${BEHAVIOR_TREE_LIB_DESTINATION}/cmake/${PROJECT_NAME}"
    )
endif()


######################################################
# EXAMPLES and TOOLS
if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
