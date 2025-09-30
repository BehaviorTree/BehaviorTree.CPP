add_library(tinyxml2 STATIC
    tinyxml2.cpp
)

add_library(tinyxml2::tinyxml2 ALIAS tinyxml2)

target_include_directories(tinyxml2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET tinyxml2
    PROPERTY
        POSITION_INDEPENDENT_CODE ON
)
