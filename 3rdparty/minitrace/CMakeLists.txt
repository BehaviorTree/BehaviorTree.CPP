add_library(minitrace STATIC
    minitrace.cpp
)

add_library(minitrace::minitrace ALIAS minitrace)

target_include_directories(minitrace
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_definitions(minitrace
    PRIVATE
        MTR_ENABLED=True
)

set_property(TARGET minitrace
    PROPERTY
        POSITION_INDEPENDENT_CODE ON
)

install(TARGETS minitrace
        EXPORT ${BTCPP_LIBRARY}Targets
        ARCHIVE DESTINATION ${BTCPP_LIB_DESTINATION}
        LIBRARY DESTINATION ${BTCPP_LIB_DESTINATION}
        RUNTIME DESTINATION ${BTCPP_BIN_DESTINATION}
        INCLUDES DESTINATION ${BTCPP_INCLUDE_DESTINATION})