cmake_minimum_required(VERSION 2.8)


add_executable(bt_log_cat         bt_log_cat.cpp )
target_link_libraries(bt_log_cat  ${BEHAVIOR_TREE_LIBRARY} )
install(TARGETS bt_log_cat  DESTINATION bin )

if( ZMQ_FOUND )
    add_executable(bt_recorder         bt_recorder.cpp )
    target_link_libraries(bt_recorder  ${BEHAVIOR_TREE_LIBRARY} )
    install(TARGETS bt_recorder  DESTINATION bin )
endif()

add_executable(bt_plugin_manifest         bt_plugin_manifest.cpp )
target_link_libraries(bt_plugin_manifest  ${BEHAVIOR_TREE_LIBRARY} )
install(TARGETS bt_plugin_manifest  DESTINATION bin )
