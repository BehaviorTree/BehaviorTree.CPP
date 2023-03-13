
include_directories(${PROJECT_SOURCE_DIR}/3rdparty)

add_executable(bt3_log_cat         bt_log_cat.cpp )
target_link_libraries(bt3_log_cat  ${BTCPP_LIBRARY} )
install(TARGETS bt3_log_cat
        DESTINATION ${BTCPP_BIN_DESTINATION} )

if( ZMQ_FOUND )
    add_executable(bt3_recorder         bt_recorder.cpp )
    target_link_libraries(bt3_recorder  ${BTCPP_LIBRARY} ${ZMQ_LIBRARIES})
    install(TARGETS bt3_recorder
            DESTINATION ${BTCPP_BIN_DESTINATION} )
endif()

add_executable(bt3_plugin_manifest         bt_plugin_manifest.cpp )
target_link_libraries(bt3_plugin_manifest  ${BTCPP_LIBRARY} )
install(TARGETS bt3_plugin_manifest
        DESTINATION ${BTCPP_BIN_DESTINATION} )



