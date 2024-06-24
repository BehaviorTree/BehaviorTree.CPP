# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

set(tests
        compiler_explorer.cpp
        parse_tree_algorithm.cpp
        parse_tree_doctest.cpp
        report_error.cpp
        shell.cpp
    )

add_executable(lexy_ext_test ${tests})
target_link_libraries(lexy_ext_test PRIVATE lexy_test_base)

