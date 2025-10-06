if(BTCPP_ENABLE_ASAN OR BTCPP_ENABLE_UBSAN OR BTCPP_ENABLE_TSAN)
    if(NOT CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
        message(FATAL_ERROR "Sanitizers require debug symbols. Please set CMAKE_BUILD_TYPE to Debug or RelWithDebInfo.")
    endif()
    add_compile_options(-fno-omit-frame-pointer)
endif()

# Address Sanitizer and Undefined Behavior Sanitizer can be run at the same time.
# Thread Sanitizer requires its own build.
if(BTCPP_ENABLE_TSAN AND (BTCPP_ENABLE_ASAN OR BTCPP_ENABLE_UBSAN))
    message(FATAL_ERROR "TSAN is not compatible with ASAN or UBSAN. Please enable only one of them.")
endif()

if(BTCPP_ENABLE_ASAN)
    message(STATUS "Address Sanitizer enabled")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(BTCPP_ENABLE_UBSAN)
    message(STATUS "Undefined Behavior Sanitizer enabled")
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

if(BTCPP_ENABLE_TSAN)
    message(STATUS "Thread Sanitizer enabled")
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()
