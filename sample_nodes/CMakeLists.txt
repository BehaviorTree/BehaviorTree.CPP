cmake_minimum_required(VERSION 2.8)

include_directories( ../include )

# compile as static libraries

set(CMAKE_DEBUG_POSTFIX "")

add_library(bt_sample_nodes STATIC
    crossdoor_nodes.cpp
    dummy_nodes.cpp
    movebase_node.cpp )

target_link_libraries(bt_sample_nodes PRIVATE ${BEHAVIOR_TREE_LIBRARY})
set_target_properties(bt_sample_nodes PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
    ${BEHAVIOR_TREE_LIB_DESTINATION} )

# to create a plugin, compile them in this way instead

add_library(crossdoor_nodes_dyn SHARED crossdoor_nodes.cpp )
target_link_libraries(crossdoor_nodes_dyn PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_compile_definitions(crossdoor_nodes_dyn PRIVATE  BT_PLUGIN_EXPORT )
set_target_properties(crossdoor_nodes_dyn PROPERTIES LIBRARY_OUTPUT_DIRECTORY
    ${BEHAVIOR_TREE_BIN_DESTINATION} )

add_library(dummy_nodes_dyn     SHARED dummy_nodes.cpp )
target_link_libraries(dummy_nodes_dyn PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_compile_definitions(dummy_nodes_dyn  PRIVATE BT_PLUGIN_EXPORT)
set_target_properties(dummy_nodes_dyn PROPERTIES LIBRARY_OUTPUT_DIRECTORY
    ${BEHAVIOR_TREE_BIN_DESTINATION} )


add_library(movebase_node_dyn   SHARED movebase_node.cpp )
target_link_libraries(movebase_node_dyn PRIVATE ${BEHAVIOR_TREE_LIBRARY})
target_compile_definitions(movebase_node_dyn PRIVATE  BT_PLUGIN_EXPORT )
set_target_properties(movebase_node_dyn PROPERTIES LIBRARY_OUTPUT_DIRECTORY
    ${BEHAVIOR_TREE_BIN_DESTINATION} )

 
