cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

add_executable(bt_log_cat         bt_log_cat.cpp )
target_link_libraries(bt_log_cat  ${BEHAVIOR_TREE_LIBRARY})

if( ZMQ_FOUND )
    add_executable(bt_recorder         bt_recorder.cpp )
    target_link_libraries(bt_recorder  ${BEHAVIOR_TREE_LIBRARY} )
endif()

add_executable(bt_plugin_manifest         bt_plugin_manifest.cpp )
target_link_libraries(bt_plugin_manifest  ${BEHAVIOR_TREE_LIBRARY} )
