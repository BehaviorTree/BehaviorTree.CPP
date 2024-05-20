
include_directories(${PROJECT_SOURCE_DIR}/3rdparty)

# add_executable(bt4_log_cat         bt_log_cat.cpp )
# target_link_libraries(bt4_log_cat  ${BTCPP_LIBRARY} )
# install(TARGETS bt4_log_cat
#         DESTINATION ${BTCPP_BIN_DESTINATION} )

if( ZMQ_FOUND )
    add_executable(bt4_recorder         bt_recorder.cpp )
    target_link_libraries(bt4_recorder  ${BTCPP_LIBRARY} ${ZMQ_LIBRARIES})
    install(TARGETS bt4_recorder
            DESTINATION ${BTCPP_BIN_DESTINATION} )
endif()

add_executable(bt4_plugin_manifest         bt_plugin_manifest.cpp )
target_link_libraries(bt4_plugin_manifest  ${BTCPP_LIBRARY} )
install(TARGETS bt4_plugin_manifest
        DESTINATION ${BTCPP_BIN_DESTINATION} )
