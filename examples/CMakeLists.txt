cmake_minimum_required(VERSION 2.8)


# This tutorial demonstrates how to compile statically a tree (no factory)
add_executable(t01_programmatic_tree         t01_programmatic_tree.cpp )
target_link_libraries(t01_programmatic_tree  dummy_nodes ${BEHAVIOR_TREE_LIBRARY}  )

# The plugin libdummy_nodes.so can be linked statically or
# loaded dynamically at run-time.
add_executable(t02_factory_static  t02_factory_tree.cpp )
target_compile_definitions(t02_factory_static PRIVATE "MANUAL_STATIC_LINKING")
target_link_libraries(t02_factory_static ${BEHAVIOR_TREE_LIBRARY}   dummy_nodes )

add_executable(t02_factory_dynamic  t02_factory_tree.cpp )
target_link_libraries(t02_factory_dynamic ${BEHAVIOR_TREE_LIBRARY}  )


# This tutorial shows how the difference between Sequence and SequenceStar
add_executable(t03_sequence_star        t03_sequence_star.cpp )
target_link_libraries(t03_sequence_star movebase_node dummy_nodes ${BEHAVIOR_TREE_LIBRARY}  )

# This tutorial demonstrates how to use blackboards and NodeParameters
add_executable(t04_blackboard         t04_blackboard.cpp )
target_link_libraries(t04_blackboard  movebase_node ${BEHAVIOR_TREE_LIBRARY}  )

add_executable(t05_crossdoor         t05_crossdoor.cpp )
target_link_libraries(t05_crossdoor  crossdoor_nodes ${BEHAVIOR_TREE_LIBRARY}  )

add_executable(t06_wrap_legacy         t06_wrap_legacy.cpp )
target_link_libraries(t06_wrap_legacy  crossdoor_nodes ${BEHAVIOR_TREE_LIBRARY}  )

add_executable(t07_include_trees  t07_include_trees.cpp )
target_link_libraries(t07_include_trees ${BEHAVIOR_TREE_LIBRARY}   dummy_nodes )

add_executable(t08_async_actions_coroutines  t08_async_actions_coroutines.cpp )
target_link_libraries(t08_async_actions_coroutines ${BEHAVIOR_TREE_LIBRARY} )
