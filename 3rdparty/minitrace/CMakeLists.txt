add_library(minitrace STATIC
    minitrace.cpp
)

add_library(minitrace::minitrace ALIAS minitrace)

target_include_directories(minitrace
    PUBLIC
        "."
)

target_compile_definitions(minitrace
    PRIVATE
        MTR_ENABLED=True
)

set_property(TARGET minitrace
    PROPERTY
        POSITION_INDEPENDENT_CODE ON
)
