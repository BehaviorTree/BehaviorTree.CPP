// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input_location.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("get_input_location()")
{
    auto verify
        = [](const auto& loc, auto line, unsigned line_nr, auto column, unsigned column_nr) {
              CHECK(loc.line_nr() == line_nr);
              CHECK(loc.column_nr() == column_nr);
              CHECK(loc.anchor()._line_begin == line);
              CHECK(loc.position() == column);
          };

    SUBCASE("code unit counting")
    {
        auto input = lexy::zstring_input("Line 1\n"
                                         "Line 2\r\n"
                                         "Line 3\n");

        auto get_location = [&](unsigned offset) {
            return lexy::get_input_location(input, input.data() + offset);
        };

        for (auto offset = 0u; offset < 7; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data(), 1, input.data() + offset, offset + 1);
        }

        for (auto offset = 7u; offset < 14; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data() + 7, 2, input.data() + offset, offset + 1 - 7);
        }

        // the \n part of the newline
        {
            auto loc = get_location(14);
            INFO(14);
            verify(loc, input.data() + 7, 2, input.data() + 13, 7);
        }

        for (auto offset = 15u; offset < 22; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data() + 15, 3, input.data() + offset, offset + 1 - 15);
        }

        // EOF
        {
            auto loc = get_location(22);
            INFO(22);
            verify(loc, input.data() + 22, 4, input.data() + 22, 1);
        }
    }
    SUBCASE("code point counting")
    {
        auto input = lexy::zstring_input<lexy::utf8_encoding>(u8"Line 1\n"
                                                              u8"Line 2\r\n"
                                                              u8"ä\n");

        auto get_location = [&](unsigned offset) {
            return lexy::get_input_location<lexy::code_point_location_counting>(input,
                                                                                input.data()
                                                                                    + offset);
        };

        for (auto offset = 0u; offset < 7; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data(), 1, input.data() + offset, offset + 1);
        }

        for (auto offset = 7u; offset < 14; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data() + 7, 2, input.data() + offset, offset + 1 - 7);
        }

        // the \n part of the newline
        {
            auto loc = get_location(14);
            INFO(14);
            verify(loc, input.data() + 7, 2, input.data() + 13, 7);
        }

        // the code units of ä
        for (auto offset = 15u; offset < 17; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data() + 15, 3, input.data() + 15, 1);
        }

        // the newline after ä
        {
            auto loc = get_location(17);
            INFO(17);
            verify(loc, input.data() + 15, 3, input.data() + 17, 2);
        }

        // EOF
        {
            auto loc = get_location(18);
            INFO(18);
            verify(loc, input.data() + 18, 4, input.data() + 18, 1);
        }
    }
    SUBCASE("byte counting")
    {
        auto input
            = lexy::zstring_input<lexy::byte_encoding>("0123456789ABCDEF0123456789ABCDEF01234");

        auto get_location = [&](unsigned offset) {
            return lexy::get_input_location(input, input.data() + offset);
        };

        for (auto offset = 0u; offset < 16; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data(), 1, input.data() + offset, offset + 1);
        }

        for (auto offset = 16u; offset < 32; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data() + 16, 2, input.data() + offset, offset + 1 - 16);
        }

        for (auto offset = 32u; offset < 36; ++offset)
        {
            auto loc = get_location(offset);
            INFO(offset);
            verify(loc, input.data() + 32, 3, input.data() + offset, offset + 1 - 32);
        }

        // EOF
        {
            auto loc = get_location(36);
            INFO(36);
            verify(loc, input.data() + 32, 3, input.data() + 36, 5);
        }
    }
}

TEST_CASE("_detail::get_input_line()")
{
    auto input = lexy::zstring_input("Line 1\n"
                                     "Line 2\r\n"
                                     "Line 3");

    auto first_line
        = lexy::_detail::get_input_line<lexy::code_unit_location_counting>(input, input.data());
    CHECK(first_line.line.begin() == input.data());
    CHECK(first_line.line.end() == input.data() + 6);
    CHECK(first_line.newline.begin() == input.data() + 6);
    CHECK(first_line.newline.end() == input.data() + 7);

    auto second_line
        = lexy::_detail::get_input_line<lexy::code_unit_location_counting>(input, input.data() + 7);
    CHECK(second_line.line.begin() == input.data() + 7);
    CHECK(second_line.line.end() == input.data() + 13);
    CHECK(second_line.newline.begin() == input.data() + 13);
    CHECK(second_line.newline.end() == input.data() + 15);

    auto third_line
        = lexy::_detail::get_input_line<lexy::code_unit_location_counting>(input,
                                                                           input.data() + 15);
    CHECK(third_line.line.begin() == input.data() + 15);
    CHECK(third_line.line.end() == input.data() + 21);
    CHECK(third_line.newline.begin() == input.data() + 21);
    CHECK(third_line.newline.end() == input.data() + 21);
}

TEST_CASE("_detail::find_cp_boundary()")
{
    SUBCASE("ASCII")
    {
        auto input = lexy::zstring_input("abc");
        auto end
            = lexy::_detail::find_cp_boundary<lexy::ascii_encoding>(input.data() + 1,
                                                                    input.data() + input.size());
        CHECK(end == input.data() + 1);
    }
    SUBCASE("UTF-8")
    {
        auto input = lexy::zstring_input<lexy::utf8_encoding>(u8"äbc");
        auto end
            = lexy::_detail::find_cp_boundary<lexy::utf8_encoding>(input.data() + 1,
                                                                   input.data() + input.size());
        CHECK(end == input.data() + 2);
    }
    SUBCASE("UTF-16")
    {
        auto input = lexy::zstring_input<lexy::utf16_encoding>(u"\U0010FFFFbc");
        auto end
            = lexy::_detail::find_cp_boundary<lexy::utf16_encoding>(input.data() + 1,
                                                                    input.data() + input.size());
        CHECK(end == input.data() + 2);
    }
}

TEST_CASE("get_input_line_annotation()")
{
    auto get_annotation = [](const auto& input, const auto& location, std::size_t size) {
        auto a = lexy::get_input_line_annotation(input, location, size);
        auto b = lexy::get_input_line_annotation(input, location, location.position() + size);
        CHECK(a.before.begin() == b.before.begin());
        CHECK(a.before.end() == b.before.end());
        CHECK(a.annotated.begin() == b.annotated.begin());
        CHECK(a.annotated.end() == b.annotated.end());
        CHECK(a.after.begin() == b.after.begin());
        CHECK(a.after.end() == b.after.end());
        CHECK(a.truncated_multiline == b.truncated_multiline);
        CHECK(a.annotated_newline == b.annotated_newline);
        CHECK(a.rounded_end == b.rounded_end);
        return a;
    };

    SUBCASE("basic")
    {
        auto input = lexy::zstring_input("0123456789\n");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = get_annotation(input, begin, 3);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 6);
        CHECK(annotation.after.begin() == input.data() + 6);
        CHECK(annotation.after.end() == input.data() + 10);
        CHECK(!annotation.truncated_multiline);
        CHECK(!annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }

    SUBCASE("empty before newline")
    {
        auto input = lexy::zstring_input("0123456789\n");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = get_annotation(input, begin, 0);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 4);
        CHECK(annotation.after.begin() == input.data() + 4);
        CHECK(annotation.after.end() == input.data() + 10);
        CHECK(!annotation.truncated_multiline);
        CHECK(!annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
    SUBCASE("empty at newline")
    {
        auto input = lexy::zstring_input("0123456789\n");

        auto begin      = lexy::get_input_location(input, input.data() + 10);
        auto annotation = get_annotation(input, begin, 0);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 10);
        CHECK(annotation.annotated.begin() == input.data() + 10);
        CHECK(annotation.annotated.end() == input.data() + 11);
        CHECK(annotation.after.begin() == input.data() + 11);
        CHECK(annotation.after.end() == input.data() + 11);
        CHECK(!annotation.truncated_multiline);
        CHECK(annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
    SUBCASE("empty after newline")
    {
        auto input = lexy::zstring_input("0123456789\n");

        auto begin      = lexy::get_input_location(input, input.data() + 11);
        auto annotation = get_annotation(input, begin, 0);
        CHECK(annotation.before.begin() == input.data() + 11);
        CHECK(annotation.before.end() == input.data() + 11);
        CHECK(annotation.annotated.begin() == input.data() + 11);
        CHECK(annotation.annotated.end() == input.data() + 11);
        CHECK(annotation.after.begin() == input.data() + 11);
        CHECK(annotation.after.end() == input.data() + 11);
        CHECK(!annotation.truncated_multiline);
        CHECK(!annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }

    SUBCASE("multiline")
    {
        auto input = lexy::zstring_input("01234\n6789");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = get_annotation(input, begin, 5);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 6);
        CHECK(annotation.after.begin() == input.data() + 6);
        CHECK(annotation.after.end() == input.data() + 6);
        CHECK(annotation.truncated_multiline);
        CHECK(annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
    SUBCASE("including newline")
    {
        auto input = lexy::zstring_input("01234\n6789");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = get_annotation(input, begin, 3);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 6);
        CHECK(annotation.after.begin() == input.data() + 6);
        CHECK(annotation.after.end() == input.data() + 6);
        CHECK(!annotation.truncated_multiline);
        CHECK(annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
    SUBCASE("only newline")
    {
        auto input = lexy::zstring_input("01234\n6789");

        auto begin      = lexy::get_input_location(input, input.data() + 5);
        auto annotation = get_annotation(input, begin, 1);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 5);
        CHECK(annotation.annotated.begin() == input.data() + 5);
        CHECK(annotation.annotated.end() == input.data() + 6);
        CHECK(annotation.after.begin() == input.data() + 6);
        CHECK(annotation.after.end() == input.data() + 6);
        CHECK(!annotation.truncated_multiline);
        CHECK(annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }

    SUBCASE("rounding")
    {
        auto input = lexy::zstring_input<lexy::utf8_encoding>(u8"0123\U0010FFFF456");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = get_annotation(input, begin, 2);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 8);
        CHECK(annotation.after.begin() == input.data() + 8);
        CHECK(annotation.after.end() == input.data() + 11);
        CHECK(!annotation.truncated_multiline);
        CHECK(!annotation.annotated_newline);
        CHECK(annotation.rounded_end);
    }

    SUBCASE("error at end without newline")
    {
        char str[] = {'0', '1', '2', '3', '4', '5'};
        auto input = lexy::string_input(str, sizeof(str));

        auto begin      = lexy::get_input_location(input, input.data() + 5);
        auto annotation = get_annotation(input, begin, 1);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 5);
        CHECK(annotation.annotated.begin() == input.data() + 5);
        CHECK(annotation.annotated.end() == input.data() + 6);
        CHECK(annotation.after.begin() == input.data() + 6);
        CHECK(annotation.after.end() == input.data() + 6);
        CHECK(!annotation.truncated_multiline);
        CHECK(!annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
    SUBCASE("error at end with newline")
    {
        char str[] = {'0', '1', '2', '3', '4', '\n'};
        auto input = lexy::string_input(str, sizeof(str));

        auto begin      = lexy::get_input_location(input, input.data() + 5);
        auto annotation = get_annotation(input, begin, 1);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 5);
        CHECK(annotation.annotated.begin() == input.data() + 5);
        CHECK(annotation.annotated.end() == input.data() + 6);
        CHECK(annotation.after.begin() == input.data() + 6);
        CHECK(annotation.after.end() == input.data() + 6);
        CHECK(!annotation.truncated_multiline);
        CHECK(annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }

    SUBCASE("clamp size without newline")
    {
        auto input = lexy::zstring_input("0123456789A");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = lexy::get_input_line_annotation(input, begin, 10);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 11);
        CHECK(annotation.after.begin() == input.data() + 11);
        CHECK(annotation.after.end() == input.data() + 11);
        CHECK(annotation.truncated_multiline);
        CHECK(!annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
    SUBCASE("clamp size with newline")
    {
        auto input = lexy::zstring_input("0123456789\n");

        auto begin      = lexy::get_input_location(input, input.data() + 3);
        auto annotation = lexy::get_input_line_annotation(input, begin, 10);
        CHECK(annotation.before.begin() == input.data());
        CHECK(annotation.before.end() == input.data() + 3);
        CHECK(annotation.annotated.begin() == input.data() + 3);
        CHECK(annotation.annotated.end() == input.data() + 11);
        CHECK(annotation.after.begin() == input.data() + 11);
        CHECK(annotation.after.end() == input.data() + 11);
        CHECK(annotation.truncated_multiline);
        CHECK(annotation.annotated_newline);
        CHECK(!annotation.rounded_end);
    }
}

