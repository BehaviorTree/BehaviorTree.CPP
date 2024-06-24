# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

# Benchmarking executable.
add_executable(lexy_benchmark_file)
target_sources(lexy_benchmark_file PRIVATE main.cpp)
target_link_libraries(lexy_benchmark_file PRIVATE foonathan::lexy::dev foonathan::lexy::file nanobench)
target_compile_definitions(lexy_benchmark_file PRIVATE LEXY_INCLUDE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../include")
set_target_properties(lexy_benchmark_file PROPERTIES OUTPUT_NAME "file")

