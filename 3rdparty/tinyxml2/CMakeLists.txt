add_library(tinyxml2 STATIC
    tinyxml2.cpp
)

add_library(tinyxml2::tinyxml2 ALIAS tinyxml2)

target_include_directories(tinyxml2
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET tinyxml2
    PROPERTY
        POSITION_INDEPENDENT_CODE ON
)

install(TARGETS tinyxml2
        EXPORT ${BTCPP_LIBRARY}Targets
        ARCHIVE DESTINATION ${BTCPP_LIB_DESTINATION}
        LIBRARY DESTINATION ${BTCPP_LIB_DESTINATION}
        RUNTIME DESTINATION ${BTCPP_BIN_DESTINATION}
        INCLUDES DESTINATION ${BTCPP_INCLUDE_DESTINATION})