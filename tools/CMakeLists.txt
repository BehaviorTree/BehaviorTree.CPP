
# add_executable(bt4_log_cat         bt_log_cat.cpp )
# target_link_libraries(bt4_log_cat  ${BTCPP_LIBRARY} )
# install(TARGETS bt4_log_cat
#         DESTINATION ${BTCPP_BIN_DESTINATION} )

# FIXME! This target doesn't build because behaviortree_cpp/flatbuffers/BT_logger_generated.h
# doesn't get generated.
# It was being silently ignored because it was included only if ZMQ_FOUND was set, but that check
# was wrong for two reasons:
# 1) The actual variable name set on FindZeroMQ.cmake is ZeroMQ_FOUND not ZMQ_FOUND
# 2) This target does not depend on ZeroMQ, but actually on its C++ wrapper cppzmq.
# Ideally we should check for cppzmq_FOUND, but because that would be only set in non-vendored mode
# and furthermore it would not be set on this scope, I chose to use BTCPP_GROOT_INTERFACE as the check
# for now.
#if( BTCPP_GROOT_INTERFACE )
#    add_executable(bt4_recorder         bt_recorder.cpp )
#    target_link_libraries(bt4_recorder  ${BTCPP_LIBRARY} cppzmq)
#    install(TARGETS bt4_recorder
#            DESTINATION ${BTCPP_BIN_DESTINATION} )
#endif()

add_executable(bt4_plugin_manifest         bt_plugin_manifest.cpp )
target_link_libraries(bt4_plugin_manifest  ${BTCPP_LIBRARY} )
install(TARGETS bt4_plugin_manifest
        DESTINATION ${BTCPP_BIN_DESTINATION} )
