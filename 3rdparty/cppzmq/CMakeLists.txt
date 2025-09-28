find_package(ZeroMQ REQUIRED)

add_library(cppzmq INTERFACE)

# This library doesn't use modern targets unfortunately.
#add_library(cppzmq::cppzmq ALIAS cppzmq)

target_include_directories(cppzmq
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(TARGET libzmq-static)
    target_link_libraries(cppzmq INTERFACE libzmq-static)
elseif(TARGET libzmq)
    target_link_libraries(cppzmq INTERFACE libzmq)
else()
    message(FATAL_ERROR "Unknown zeromq target name")
endif()
