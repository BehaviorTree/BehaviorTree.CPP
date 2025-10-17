add_library(flatbuffers INTERFACE)

add_library(flatbuffers::flatbuffers ALIAS flatbuffers)

target_include_directories(flatbuffers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

install(TARGETS flatbuffers
        EXPORT ${BTCPP_LIBRARY}Targets
        ARCHIVE DESTINATION ${BTCPP_LIB_DESTINATION}
        LIBRARY DESTINATION ${BTCPP_LIB_DESTINATION}
        RUNTIME DESTINATION ${BTCPP_BIN_DESTINATION}
        INCLUDES DESTINATION ${BTCPP_INCLUDE_DESTINATION})
