add_library(minicoro INTERFACE)

add_library(minicoro::minicoro ALIAS minicoro)

target_include_directories(minicoro
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
