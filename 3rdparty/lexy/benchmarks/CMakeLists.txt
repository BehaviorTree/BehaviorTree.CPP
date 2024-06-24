# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

# Fetch nanobench.
message(STATUS "Fetching nanobench")
include(FetchContent)
FetchContent_Declare(nanobench URL https://github.com/martinus/nanobench/archive/v4.3.0.zip)
FetchContent_MakeAvailable(nanobench)

add_subdirectory(json)
add_subdirectory(file)

