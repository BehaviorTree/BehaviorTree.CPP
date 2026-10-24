cmake_minimum_required(VERSION 3.16.3)

project(btcpp_parent VERSION 9.8.7 LANGUAGES C CXX)

if(NOT BTCPP_SOURCE_DIR OR NOT BTCPP_EXPECTED_VERSION)
    message(FATAL_ERROR "BTCPP_SOURCE_DIR and BTCPP_EXPECTED_VERSION are required")
endif()

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BTCPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BTCPP_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BTCPP_GROOT_INTERFACE OFF CACHE BOOL "" FORCE)
set(BTCPP_SQLITE_LOGGING OFF CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_ament_cmake TRUE CACHE BOOL "" FORCE)

add_subdirectory("${BTCPP_SOURCE_DIR}" behaviortree_cpp)

get_target_property(btcpp_compile_definitions behaviortree_cpp COMPILE_DEFINITIONS)
set(expected_definition "BTCPP_LIBRARY_VERSION=\"${BTCPP_EXPECTED_VERSION}\"")
list(FIND btcpp_compile_definitions "${expected_definition}" expected_version_index)

if(expected_version_index EQUAL -1)
    message(FATAL_ERROR
        "Expected ${expected_definition}, got: ${btcpp_compile_definitions}")
endif()
