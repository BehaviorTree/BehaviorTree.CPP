cmake_minimum_required(VERSION 2.8)
project(behavior_tree_core)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Werror=return-type")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


include_directories( ../include ../sample_nodes )

# This tutorial demonstrates how to compile statically a tree (no factory)
add_executable(t01_programmatic_tree         t01_programmatic_tree.cpp )
target_link_libraries(t01_programmatic_tree  dummy_nodes ${BEHAVIOR_TREE_LIBRARIES} )

# The plugin libdummy_nodes.so can be linked statically or
# loaded dynamically at run-time.
add_executable(t02_factory_static  t02_factory_tree.cpp )
target_compile_definitions(t02_factory_static PRIVATE "MANUAL_STATIC_LINKING")
target_link_libraries(t02_factory_static ${BEHAVIOR_TREE_LIBRARIES}  dummy_nodes )

add_executable(t02_factory_dynamic  t02_factory_tree.cpp )
target_link_libraries(t02_factory_dynamic ${BEHAVIOR_TREE_LIBRARIES} )


# This tutorial shows how the difference between Sequence and SequenceStar
add_executable(t03_sequence_star        t03_sequence_star.cpp )
target_link_libraries(t03_sequence_star movebase_node dummy_nodes ${BEHAVIOR_TREE_LIBRARIES} )

# This tutorial demonstrates how to use blackboards and NodeParameters
add_executable(t04_blackboard         t04_blackboard.cpp )
target_link_libraries(t04_blackboard  movebase_node ${BEHAVIOR_TREE_LIBRARIES} )


add_executable(crossdoor_example         crossdoor_example.cpp )
target_link_libraries(crossdoor_example  crossdoor_nodes ${BEHAVIOR_TREE_LIBRARIES} )
