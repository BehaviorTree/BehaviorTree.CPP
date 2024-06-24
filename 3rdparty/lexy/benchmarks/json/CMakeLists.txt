# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

include(FetchContent)

# Fetch benchmark data.
message(STATUS "Fetching json benchmark data")
function(fetch_data file url)
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/data/${file})
        file(DOWNLOAD "${url}" "${CMAKE_CURRENT_BINARY_DIR}/data/${file}" STATUS status)
        list(GET status 0 status_code)
        list(GET status 1 status_string)

        if(NOT status_code EQUAL 0)
            message(FATAL_ERROR "error downloading ${file}: ${status_string}")
        endif()
    endif()
endfunction()

fetch_data(canada.json https://github.com/miloyip/nativejson-benchmark/raw/master/data/canada.json)
fetch_data(citm_catalog.json https://github.com/miloyip/nativejson-benchmark/raw/master/data/citm_catalog.json)
fetch_data(twitter.json https://raw.githubusercontent.com/miloyip/nativejson-benchmark/master/data/twitter.json)

# Benchmarking executable.
add_executable(lexy_benchmark_json)
target_sources(lexy_benchmark_json PRIVATE main.cpp baseline.cpp lexy.cpp)
target_link_libraries(lexy_benchmark_json PRIVATE foonathan::lexy::dev foonathan::lexy::file nanobench)
target_compile_definitions(lexy_benchmark_json PRIVATE LEXY_BENCHMARK_DATA="${CMAKE_CURRENT_BINARY_DIR}/data/")
set_target_properties(lexy_benchmark_json PROPERTIES OUTPUT_NAME "json")

# Compare with PEGTL's json parser.
message(STATUS "Fetching PEGTL")
FetchContent_Declare(pegtl URL https://github.com/taocpp/PEGTL/archive/main.zip)
if(NOT pegtl_POPULATED)
    FetchContent_Populate(pegtl)
    add_subdirectory(${pegtl_SOURCE_DIR} ${pegtl_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_sources(lexy_benchmark_json PRIVATE pegtl.cpp)
target_link_libraries(lexy_benchmark_json PRIVATE taocpp::pegtl)

# Compare with nlohmann/json.
message(STATUS "Fetching nlohmann/json")
FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/archive/develop.zip)
FetchContent_GetProperties(nlohmann_json)
if(NOT nlohmann_json_POPULATED)
    FetchContent_Populate(nlohmann_json)
endif()

target_sources(lexy_benchmark_json PRIVATE nlohmann.cpp)
target_include_directories(lexy_benchmark_json PRIVATE ${nlohmann_json_SOURCE_DIR}/include)

# Compare with rapidjson.
message(STATUS "Fetching rapidjson")
FetchContent_Declare(rapidjson URL https://github.com/Tencent/rapidjson/archive/master.zip)
FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()

target_sources(lexy_benchmark_json PRIVATE rapidjson.cpp)
target_include_directories(lexy_benchmark_json PRIVATE ${rapidjson_SOURCE_DIR}/include)

# Compare with Boost.JSON.
find_package(Boost COMPONENTS json)
if (Boost_FOUND)
    target_sources(lexy_benchmark_json PRIVATE boost.cpp)
    target_link_libraries(lexy_benchmark_json PRIVATE Boost::json)
    target_compile_definitions(lexy_benchmark_json PRIVATE LEXY_HAS_BOOST_JSON)
endif()

