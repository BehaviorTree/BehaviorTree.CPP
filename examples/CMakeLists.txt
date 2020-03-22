cmake_minimum_required(VERSION 3.5.2)

include_directories( ../sample_nodes )

set(CMAKE_DEBUG_POSTFIX "")

# The plugin libdummy_nodes.so can be linked statically or
# loaded dynamically at run-time.
add_executable(t01_first_tree_static  t01_build_your_first_tree.cpp )
target_compile_definitions(t01_first_tree_static PRIVATE "MANUAL_STATIC_LINKING")
target_link_libraries(t01_first_tree_static ${BEHAVIOR_TREE_LIBRARY}   bt_sample_nodes )

add_executable(t01_first_tree_dynamic  t01_build_your_first_tree.cpp )
target_link_libraries(t01_first_tree_dynamic ${BEHAVIOR_TREE_LIBRARY}  )


add_executable(t02_basic_ports  t02_basic_ports.cpp )
target_link_libraries(t02_basic_ports ${BEHAVIOR_TREE_LIBRARY} bt_sample_nodes )

add_executable(t03_generic_ports        t03_generic_ports.cpp )
target_link_libraries(t03_generic_ports ${BEHAVIOR_TREE_LIBRARY}  bt_sample_nodes  )

add_executable(t04_reactive_sequence         t04_reactive_sequence.cpp )
target_link_libraries(t04_reactive_sequence  ${BEHAVIOR_TREE_LIBRARY} bt_sample_nodes  )

add_executable(t05_cross_door         t05_crossdoor.cpp )
target_link_libraries(t05_cross_door   ${BEHAVIOR_TREE_LIBRARY} bt_sample_nodes )

add_executable(t06_subtree_port_remapping  t06_subtree_port_remapping.cpp )
target_link_libraries(t06_subtree_port_remapping ${BEHAVIOR_TREE_LIBRARY}   bt_sample_nodes )

add_executable(t07_wrap_legacy         t07_wrap_legacy.cpp )
target_link_libraries(t07_wrap_legacy   ${BEHAVIOR_TREE_LIBRARY} )

add_executable(t08_additional_node_args   t08_additional_node_args.cpp )
target_link_libraries(t08_additional_node_args   ${BEHAVIOR_TREE_LIBRARY} )

if (BT_COROUTINES)
    add_executable(t09_async_actions_coroutines  t09_async_actions_coroutines.cpp )
    target_link_libraries(t09_async_actions_coroutines ${BEHAVIOR_TREE_LIBRARY} )
endif()

add_executable(t10_include_trees  t10_include_trees.cpp )
target_link_libraries(t10_include_trees  ${BEHAVIOR_TREE_LIBRARY} bt_sample_nodes )


add_executable(t11_runtime_ports  t11_runtime_ports.cpp )
target_link_libraries(t11_runtime_ports  ${BEHAVIOR_TREE_LIBRARY} bt_sample_nodes )
