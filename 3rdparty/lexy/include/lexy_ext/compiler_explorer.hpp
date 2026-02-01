// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED
#define LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/buffer_builder.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/input/buffer.hpp>

namespace lexy_ext
{
/// Input that uses Compiler Explorer's stdin.
/// It consumes the entire stdin, which is then at EOF.
lexy::buffer<lexy::utf8_encoding> compiler_explorer_input()
{
    // We can't use ftell() to get file size
    // So instead use a conservative loop.
    lexy::_detail::buffer_builder<char> builder;
    while (true)
    {
        const auto buffer_size = builder.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from stdin,
        // commiting what we've just read.
        const auto read = std::fread(builder.write_data(), sizeof(char), buffer_size, stdin);
        builder.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            // We should have reached the end.
            LEXY_ASSERT(!std::ferror(stdin), "read error");
            LEXY_ASSERT(std::feof(stdin), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        builder.grow();
    }

    return lexy::buffer<lexy::utf8_encoding>(builder.read_data(), builder.read_size());
}

//=== convenience typedefs ===//
using compiler_explorer_lexeme = lexy::buffer_lexeme<lexy::utf8_encoding>;

template <typename Tag>
using compiler_explorer_error = lexy::buffer_error<Tag, lexy::utf8_encoding>;

template <typename Production>
using compiler_explorer_error_context = lexy::buffer_error_context<Production, lexy::utf8_encoding>;
} // namespace lexy_ext

#endif // LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED

