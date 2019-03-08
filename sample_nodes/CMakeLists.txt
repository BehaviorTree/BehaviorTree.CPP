cmake_minimum_required(VERSION 2.8)

include_directories( ../include )

add_library(crossdoor_nodes STATIC crossdoor_nodes.cpp )
target_link_libraries(crossdoor_nodes PRIVATE ${BEHAVIOR_TREE_LIBRARY})

add_library(dummy_nodes     STATIC dummy_nodes.cpp )
target_link_libraries(dummy_nodes PRIVATE ${BEHAVIOR_TREE_LIBRARY})

add_library(movebase_node   STATIC movebase_node.cpp )
target_link_libraries(movebase_node PRIVATE ${BEHAVIOR_TREE_LIBRARY})



add_library(crossdoor_nodes_dyn SHARED crossdoor_nodes.cpp )
target_link_libraries(crossdoor_nodes_dyn PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_compile_definitions(crossdoor_nodes_dyn PRIVATE  WIN_EXPORT )

add_library(dummy_nodes_dyn     SHARED dummy_nodes.cpp )
target_link_libraries(dummy_nodes_dyn PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_compile_definitions(dummy_nodes_dyn  PRIVATE WIN_EXPORT)

add_library(movebase_node_dyn   SHARED movebase_node.cpp )
target_link_libraries(movebase_node_dyn PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_compile_definitions(movebase_node_dyn PRIVATE  WIN_EXPORT )
 