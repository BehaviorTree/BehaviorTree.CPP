add_library(wildcards INTERFACE)

add_library(wildcards::wildcards ALIAS wildcards)

target_include_directories(wildcards
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
