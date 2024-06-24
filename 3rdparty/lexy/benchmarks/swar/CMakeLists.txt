# Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

# Benchmarking executable.
add_executable(lexy_benchmark_swar)
target_sources(lexy_benchmark_swar PRIVATE main.cpp swar.hpp any.cpp delimited.cpp digits.cpp identifier.cpp literal.cpp until.cpp)
target_link_libraries(lexy_benchmark_swar PRIVATE foonathan::lexy::dev foonathan::lexy::file foonathan::lexy::unicode nanobench)
set_target_properties(lexy_benchmark_swar PROPERTIES OUTPUT_NAME "swar")

