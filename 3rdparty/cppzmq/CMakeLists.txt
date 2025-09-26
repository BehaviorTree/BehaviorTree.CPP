add_library(cppzmq INTERFACE)

# This library doesn't use modern targets unfortunately.
#add_library(cppzmq::cppzmq ALIAS cppzmq)

target_include_directories(cppzmq
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
