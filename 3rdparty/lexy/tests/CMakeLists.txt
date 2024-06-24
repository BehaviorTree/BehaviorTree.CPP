# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

# Fetch doctest.
message(STATUS "Fetching doctest")
include(FetchContent)
FetchContent_Declare(doctest URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.9.zip)
FetchContent_MakeAvailable(doctest)

# A generic test target.
add_library(lexy_test_base ${CMAKE_CURRENT_SOURCE_DIR}/doctest_main.cpp)
target_link_libraries(lexy_test_base PUBLIC foonathan::lexy::dev foonathan::lexy::file foonathan::lexy::unicode doctest)
target_compile_definitions(lexy_test_base PUBLIC LEXY_TEST)

if(MSVC AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    target_compile_options(lexy_test_base PUBLIC
        /Zc:preprocessor    # need a conforming preprocessor for stringifying escape characters
        "/utf-8"            # use utf-8 instead of current code page
    )
endif()

# Add the individual tests.
add_subdirectory(lexy)
add_subdirectory(lexy_ext)
add_subdirectory(examples)
add_subdirectory(playground)

add_test(NAME lexy_test COMMAND lexy_test)
add_test(NAME lexy_ext_test COMMAND lexy_ext_test)
add_test(NAME email COMMAND lexy_test_email)
add_test(NAME ip COMMAND lexy_test_ip_address)
add_test(NAME json COMMAND lexy_test_json)
add_test(NAME shell COMMAND lexy_test_shell)
add_test(NAME xml COMMAND lexy_test_xml)
add_test(NAME turing COMMAND lexy_test_turing)

