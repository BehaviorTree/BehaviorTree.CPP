# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 2022.05.1 LANGUAGES CXX)

set(LEXY_USER_CONFIG_HEADER "" CACHE FILEPATH "The user config header for lexy.")
option(LEXY_FORCE_CPP17     "Whether or not lexy should use C++17 even if compiler supports C++20." OFF)

add_subdirectory(src)

