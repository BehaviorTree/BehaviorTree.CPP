cmake_minimum_required(VERSION 2.8)

include_directories( ../include )

add_library(crossdoor_nodes SHARED crossdoor_nodes.cpp )
target_link_libraries(crossdoor_nodes PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_include_directories(crossdoor_nodes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(dummy_nodes     SHARED dummy_nodes.cpp )
target_link_libraries(dummy_nodes PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_include_directories(dummy_nodes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(movebase_node   SHARED movebase_node.cpp )
target_link_libraries(movebase_node PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_include_directories(movebase_node PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
