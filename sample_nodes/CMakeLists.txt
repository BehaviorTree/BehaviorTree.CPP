cmake_minimum_required(VERSION 2.8)
project(behavior_tree_core)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Werror=return-type")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories( ../include )

add_library(crossdoor_nodes SHARED crossdoor_nodes.cpp )
add_library(dummy_nodes     SHARED dummy_nodes.cpp )
add_library(movebase_node   SHARED movebase_node.cpp )
