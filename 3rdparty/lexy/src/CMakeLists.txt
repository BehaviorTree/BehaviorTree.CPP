# Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

get_filename_component(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/lexy ABSOLUTE)
get_filename_component(ext_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/lexy_ext ABSOLUTE)
set(header_files
        ${include_dir}/_detail/any_ref.hpp
        ${include_dir}/_detail/assert.hpp
        ${include_dir}/_detail/buffer_builder.hpp
        ${include_dir}/_detail/code_point.hpp
        ${include_dir}/_detail/config.hpp
        ${include_dir}/_detail/detect.hpp
        ${include_dir}/_detail/integer_sequence.hpp
        ${include_dir}/_detail/invoke.hpp
        ${include_dir}/_detail/iterator.hpp
        ${include_dir}/_detail/lazy_init.hpp
        ${include_dir}/_detail/memory_resource.hpp
        ${include_dir}/_detail/nttp_string.hpp
        ${include_dir}/_detail/stateless_lambda.hpp
        ${include_dir}/_detail/std.hpp
        ${include_dir}/_detail/string_view.hpp
        ${include_dir}/_detail/swar.hpp
        ${include_dir}/_detail/tuple.hpp
        ${include_dir}/_detail/type_name.hpp

        ${include_dir}/action/base.hpp
        ${include_dir}/action/match.hpp
        ${include_dir}/action/parse.hpp
        ${include_dir}/action/parse_as_tree.hpp
        ${include_dir}/action/scan.hpp
        ${include_dir}/action/validate.hpp

        ${include_dir}/callback/adapter.hpp
        ${include_dir}/callback/aggregate.hpp
        ${include_dir}/callback/base.hpp
        ${include_dir}/callback/bind.hpp
        ${include_dir}/callback/bit_cast.hpp
        ${include_dir}/callback/composition.hpp
        ${include_dir}/callback/constant.hpp
        ${include_dir}/callback/container.hpp
        ${include_dir}/callback/fold.hpp
        ${include_dir}/callback/forward.hpp
        ${include_dir}/callback/integer.hpp
        ${include_dir}/callback/noop.hpp
        ${include_dir}/callback/object.hpp
        ${include_dir}/callback/string.hpp

        ${include_dir}/dsl/any.hpp
        ${include_dir}/dsl/ascii.hpp
        ${include_dir}/dsl/base.hpp
        ${include_dir}/dsl/bits.hpp
        ${include_dir}/dsl/bom.hpp
        ${include_dir}/dsl/brackets.hpp
        ${include_dir}/dsl/branch.hpp
        ${include_dir}/dsl/byte.hpp
        ${include_dir}/dsl/capture.hpp
        ${include_dir}/dsl/case_folding.hpp
        ${include_dir}/dsl/char_class.hpp
        ${include_dir}/dsl/choice.hpp
        ${include_dir}/dsl/code_point.hpp
        ${include_dir}/dsl/combination.hpp
        ${include_dir}/dsl/context_counter.hpp
        ${include_dir}/dsl/context_flag.hpp
        ${include_dir}/dsl/context_identifier.hpp
        ${include_dir}/dsl/delimited.hpp
        ${include_dir}/dsl/digit.hpp
        ${include_dir}/dsl/effect.hpp
        ${include_dir}/dsl/eof.hpp
        ${include_dir}/dsl/error.hpp
        ${include_dir}/dsl/expression.hpp
        ${include_dir}/dsl/follow.hpp
        ${include_dir}/dsl/flags.hpp
        ${include_dir}/dsl/identifier.hpp
        ${include_dir}/dsl/if.hpp
        ${include_dir}/dsl/integer.hpp
        ${include_dir}/dsl/lookahead.hpp
        ${include_dir}/dsl/loop.hpp
        ${include_dir}/dsl/member.hpp
        ${include_dir}/dsl/newline.hpp
        ${include_dir}/dsl/option.hpp
        ${include_dir}/dsl/operator.hpp
        ${include_dir}/dsl/parse_as.hpp
        ${include_dir}/dsl/parse_tree_node.hpp
        ${include_dir}/dsl/peek.hpp
        ${include_dir}/dsl/position.hpp
        ${include_dir}/dsl/production.hpp
        ${include_dir}/dsl/punctuator.hpp
        ${include_dir}/dsl/recover.hpp
        ${include_dir}/dsl/repeat.hpp
        ${include_dir}/dsl/return.hpp
        ${include_dir}/dsl/scan.hpp
        ${include_dir}/dsl/separator.hpp
        ${include_dir}/dsl/sequence.hpp
        ${include_dir}/dsl/sign.hpp
        ${include_dir}/dsl/subgrammar.hpp
        ${include_dir}/dsl/symbol.hpp
        ${include_dir}/dsl/terminator.hpp
        ${include_dir}/dsl/times.hpp
        ${include_dir}/dsl/token.hpp
        ${include_dir}/dsl/unicode.hpp
        ${include_dir}/dsl/until.hpp
        ${include_dir}/dsl/whitespace.hpp

        ${include_dir}/input/argv_input.hpp
        ${include_dir}/input/base.hpp
        ${include_dir}/input/buffer.hpp
        ${include_dir}/input/file.hpp
        ${include_dir}/input/lexeme_input.hpp
        ${include_dir}/input/parse_tree_input.hpp
        ${include_dir}/input/range_input.hpp
        ${include_dir}/input/string_input.hpp

        ${include_dir}/callback.hpp
        ${include_dir}/code_point.hpp
        ${include_dir}/dsl.hpp
        ${include_dir}/encoding.hpp
        ${include_dir}/error.hpp
        ${include_dir}/grammar.hpp
        ${include_dir}/input_location.hpp
        ${include_dir}/lexeme.hpp
        ${include_dir}/parse_tree.hpp
        ${include_dir}/token.hpp
        ${include_dir}/visualize.hpp
        PARENT_SCOPE)
set(ext_header_files
        ${ext_include_dir}/compiler_explorer.hpp
        ${ext_include_dir}/parse_tree_algorithm.hpp
        ${ext_include_dir}/parse_tree_doctest.hpp
        ${ext_include_dir}/report_error.hpp
        ${ext_include_dir}/shell.hpp
        PARENT_SCOPE)

# Base target for common options.
add_library(_lexy_base INTERFACE)
target_sources(_lexy_base INTERFACE ${header_files})
if(LEXY_FORCE_CPP17)
    target_compile_features(_lexy_base INTERFACE cxx_std_17)
else()
    if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        target_compile_features(_lexy_base INTERFACE cxx_std_20)
    else()
        target_compile_features(_lexy_base INTERFACE cxx_std_17)
    endif()
endif()

if (LEXY_USER_CONFIG_HEADER)
    if(EXISTS ${LEXY_USER_CONFIG_HEADER})
        target_compile_definitions(_lexy_base INTERFACE LEXY_USER_CONFIG_HEADER="${LEXY_USER_CONFIG_HEADER}")
    else()
        message(WARNING "User config header for lexy specified as '${LEXY_USER_CONFIG_HEADER}', but not found.")
    endif()
endif()

function(add_alias name target)
    add_library(foonathan::${name} ALIAS ${target})
    set_target_properties(${target} PROPERTIES EXPORT_NAME ${name})
endfunction()

include(GNUInstallDirs)

# Core library.
add_library(lexy_core INTERFACE)
add_alias(lexy::core lexy_core)
target_link_libraries(lexy_core INTERFACE _lexy_base)
target_include_directories(lexy_core SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# Core library with warnings; for development only.
add_library(lexy_dev INTERFACE)
add_alias(lexy::dev lexy_dev)
target_link_libraries(lexy_dev INTERFACE _lexy_base)
target_include_directories(lexy_dev INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        target_compile_options(lexy_dev INTERFACE /WX /W3 /D _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(lexy_dev INTERFACE -pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    endif()
    # clang doesn't like operator precedence rules we're using for the DSL.
    target_compile_options(lexy_dev INTERFACE -Wno-shift-op-parentheses -Wno-parentheses-equality)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(lexy_dev INTERFACE -pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    # GCC doesn't like the operator precedence rules we're using for the DSL.
    target_compile_options(lexy_dev INTERFACE -Wno-parentheses)
    # GCC doesn't properly detect when local typedefs escape via the return type.
    target_compile_options(lexy_dev INTERFACE -Wno-unused-local-typedefs)
    # GCC's arry bounds, maybe uninitialized, and restrict warning seems to have false positives.
    target_compile_options(lexy_dev INTERFACE -Wno-array-bounds -Wno-maybe-uninitialized -Wno-restrict)
elseif(MSVC)
    target_compile_options(lexy_dev INTERFACE /WX /W3 /D _CRT_SECURE_NO_WARNINGS /wd5105 /utf-8)
endif()

# Link to have FILE I/O.
add_library(lexy_file STATIC)
add_alias(lexy::file lexy_file)
target_link_libraries(lexy_file PRIVATE foonathan::lexy::dev)
target_sources(lexy_file PRIVATE input/file.cpp)

# Link to enable unicode database.
add_library(lexy_unicode INTERFACE)
add_alias(lexy::unicode lexy_unicode)
target_compile_definitions(lexy_unicode INTERFACE LEXY_HAS_UNICODE_DATABASE=1)

# Link to have extension headers.
add_library(lexy_ext INTERFACE)
add_alias(lexy::ext lexy_ext)
target_sources(lexy_ext INTERFACE ${ext_headers_files})

# Umbrella target with all components.
add_library(lexy INTERFACE)
add_alias(lexy lexy)
target_link_libraries(lexy INTERFACE foonathan::lexy::core foonathan::lexy::file foonathan::lexy::unicode foonathan::lexy::ext)

# Link to enable experimental features.
add_library(lexy_experimental INTERFACE)
add_alias(lexy::experimental lexy_experimental)
target_compile_definitions(lexy_experimental INTERFACE LEXY_EXPERIMENTAL=1)

