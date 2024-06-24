// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/visualize.hpp>

#include <doctest/doctest.h>
#include <iterator>
#include <lexy/input/string_input.hpp>
#include <lexy/parse_tree.hpp>
#include <string>

TEST_CASE("visualize code_point")
{
    auto visualize = [](lexy::code_point cp, lexy::visualization_options opts) {
        std::string str;
        lexy::visualize_to(std::back_insert_iterator(str), cp, opts);
        return str;
    };

    SUBCASE("default")
    {
        auto opts = lexy::visualization_options{};

        CHECK(visualize(lexy::code_point(), opts) == R"(\u????)");

        CHECK(visualize(lexy::code_point('\0'), opts) == R"(\0)");
        CHECK(visualize(lexy::code_point('\n'), opts) == R"(\n)");
        CHECK(visualize(lexy::code_point('\r'), opts) == R"(\r)");
        CHECK(visualize(lexy::code_point('\t'), opts) == R"(\t)");
        CHECK(visualize(lexy::code_point('\x01'), opts) == R"(\u0001)");
        CHECK(visualize(lexy::code_point('\x02'), opts) == R"(\u0002)");
        CHECK(visualize(lexy::code_point('\x7F'), opts) == R"(\u007F)");

        CHECK(visualize(lexy::code_point(' '), opts) == R"( )");
        CHECK(visualize(lexy::code_point(' '), opts | lexy::visualize_space) == R"(\u0020)");

        CHECK(visualize(lexy::code_point('\\'), opts) == R"(\\)");

        CHECK(visualize(lexy::code_point('a'), opts) == R"(a)");
        CHECK(visualize(lexy::code_point('B'), opts) == R"(B)");

        CHECK(visualize(lexy::code_point(0x80), opts) == R"(\u0080)");

        CHECK(visualize(lexy::code_point(0x1234), opts) == R"(\u1234)");
        CHECK(visualize(lexy::code_point(0x10'FFFF), opts) == R"(\U0010FFFF)");
    }
    SUBCASE("unicode")
    {
        auto opts = lexy::visualization_options{lexy::visualize_use_unicode};

        CHECK(visualize(lexy::code_point(), opts) == R"(⟨U+????⟩)");

        CHECK(visualize(lexy::code_point('\0'), opts) == R"(⟨NUL⟩)");
        CHECK(visualize(lexy::code_point('\n'), opts) == R"(⟨LF⟩)");
        CHECK(visualize(lexy::code_point('\r'), opts) == R"(⟨CR⟩)");
        CHECK(visualize(lexy::code_point('\t'), opts) == R"(⟨HT⟩)");
        CHECK(visualize(lexy::code_point('\x01'), opts) == R"(⟨U+0001⟩)");
        CHECK(visualize(lexy::code_point('\x02'), opts) == R"(⟨U+0002⟩)");
        CHECK(visualize(lexy::code_point('\x7F'), opts) == R"(⟨U+007F⟩)");

        CHECK(visualize(lexy::code_point(' '), opts) == R"( )");
        CHECK(visualize(lexy::code_point(' '), opts | lexy::visualize_space) == R"(⟨SP⟩)");

        CHECK(visualize(lexy::code_point('\\'), opts) == R"(\)");

        CHECK(visualize(lexy::code_point('a'), opts) == R"(a)");
        CHECK(visualize(lexy::code_point('B'), opts) == R"(B)");

        CHECK(visualize(lexy::code_point(0x80), opts) == R"(⟨U+0080⟩)");

        CHECK(visualize(lexy::code_point(0x1234), opts) == R"(⟨U+1234⟩)");
        CHECK(visualize(lexy::code_point(0x10'FFFF), opts) == R"(⟨U+10FFFF⟩)");
    }

    SUBCASE("symbols")
    {
        auto opts = lexy::visualization_options{lexy::visualize_use_symbols};

        CHECK(visualize(lexy::code_point('\n'), opts) == R"(⏎)");
        CHECK(visualize(lexy::code_point('\t'), opts) == R"(⇨)");

        CHECK(visualize(lexy::code_point(' '), opts) == R"( )");
        CHECK(visualize(lexy::code_point(' '), opts | lexy::visualize_space) == R"(␣)");
    }

    SUBCASE("tab as spaces")
    {
        auto opts      = lexy::visualization_options{};
        opts.tab_width = 4;

        CHECK(visualize(lexy::code_point('\t'), opts) == R"(    )");
    }
}

TEST_CASE("visualize lexeme")
{
    auto visualize = [](auto encoding, const auto* str, unsigned char limit = 0) {
        auto input   = lexy::zstring_input<decltype(encoding)>(str);
        using lexeme = lexy::lexeme_for<decltype(input)>;

        std::string result;

        lexy::visualization_options opts{};
        opts.max_lexeme_width = limit;
        lexy::visualize_to(std::back_insert_iterator(result),
                           lexeme(input.data(), input.data() + input.size()), opts);

        return result;
    };

    SUBCASE("default/ascii encoding")
    {
        CHECK(visualize(lexy::default_encoding{}, "abc") == R"(abc)");
        CHECK(visualize(lexy::default_encoding{}, "\n\t\\") == R"(\n\t\\)");

        char out_of_range[] = {'a', char(0xFF), 'c', '\0'};
        CHECK(visualize(lexy::default_encoding{}, out_of_range) == R"(a\xFFc)");

        CHECK(visualize(lexy::default_encoding{}, "abc", 2) == R"(ab...)");
    }
    SUBCASE("unicode encoding")
    {
        CHECK(visualize(lexy::utf8_encoding{}, "abc") == R"(abc)");
        CHECK(visualize(lexy::utf8_encoding{}, "\n\t\\") == R"(\n\t\\)");
        CHECK(visualize(lexy::utf8_encoding{}, "\u1234") == R"(\u1234)");
        CHECK(visualize(lexy::utf8_encoding{}, "\xC0\xA0") == R"(\xC0\xA0)");

        CHECK(visualize(lexy::utf16_encoding{}, u"abc") == R"(abc)");
        CHECK(visualize(lexy::utf16_encoding{}, u"\n\t\\") == R"(\n\t\\)");
        CHECK(visualize(lexy::utf16_encoding{}, u"\u1234") == R"(\u1234)");
        CHECK(visualize(lexy::utf16_encoding{}, u"\xD811") == R"(\xD8\x11)");

        CHECK(visualize(lexy::utf32_encoding{}, U"abc") == R"(abc)");
        CHECK(visualize(lexy::utf32_encoding{}, U"\n\t\\") == R"(\n\t\\)");
        CHECK(visualize(lexy::utf32_encoding{}, U"\u1234") == R"(\u1234)");
        CHECK(visualize(lexy::utf32_encoding{}, U"\x1100FF") == R"(\x11\x00\xFF)");

        CHECK(visualize(lexy::utf8_encoding{}, "abc", 2) == R"(ab...)");
    }
    SUBCASE("byte encoding")
    {
        CHECK(visualize(lexy::byte_encoding{}, "abc") == R"(\61\62\63)");
        CHECK(visualize(lexy::byte_encoding{}, "\n\t\\") == R"(\0A\09\5C)");
        CHECK(visualize(lexy::byte_encoding{}, "\x11\x42") == R"(\11\42)");

        CHECK(visualize(lexy::byte_encoding{}, "abc", 2) == R"(\61\62...)");
    }
}

namespace
{
enum class token_kind
{
    a,
    b,
    c,
};

const char* token_kind_name(token_kind k)
{
    switch (k)
    {
    case token_kind::a:
        return "a";
    case token_kind::b:
        return "b";
    case token_kind::c:
        return "c";
    }

    return "";
}

struct child_p : lexy::token_production
{
    static constexpr auto name = "child_p";
};
struct root_p
{
    static constexpr auto name = "root_p";
};
} // namespace

TEST_CASE("visualize parse_tree")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc \n\x84)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child     = builder.start_production(child_p{});
        auto sub_child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 10);
        builder.token(token_kind::b, input.data() + 10, input.data() + 11);
        builder.finish_production(LEXY_MOV(sub_child));
        builder.finish_production(LEXY_MOV(child));

        child = builder.start_production(child_p{});
        builder.token(token_kind::a, input.data() + 11, input.data() + 14);
        builder.finish_production(LEXY_MOV(child));

        builder.token(lexy::eof_token_kind, input.data() + 14, input.data() + 14);

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    auto visualize = [](const parse_tree& tree, auto opts) {
        std::string str;
        lexy::visualize_to(std::back_insert_iterator(str), tree, {opts});
        return str;
    };

    SUBCASE("default")
    {
        auto flags = lexy::visualize_default;
        CHECK(visualize(tree, flags) == R"*(root_p:
- a: 123
- child_p:
  - child_p:
    - b: (
    - c: abc\u0020\n\x84
    - b: )
- child_p:
  - a: 321
- EOF
)*");
    }
    SUBCASE("unicode")
    {
        auto flags = lexy::visualize_use_unicode;
        CHECK(visualize(tree, flags) == R"*(root_p:
├──a: 123
├──child_p:
│  └──child_p:
│     ├──b: (
│     ├──c: abc⟨SP⟩⟨LF⟩⟨0x84⟩
│     └──b: )
├──child_p:
│  └──a: 321
└──EOF
)*");
    }

    SUBCASE("depth-limited")
    {
        auto opts = lexy::visualization_options{{}, 2};
        CHECK(visualize(tree, opts) == R"*(root_p:
- a: 123
- child_p:
  - child_p: ...
- child_p:
  - a: 321
- EOF
)*");
    }
    SUBCASE("depth-limited unicode")
    {
        auto opts = lexy::visualization_options{lexy::visualize_use_unicode, 2};
        CHECK(visualize(tree, opts) == R"*(root_p:
├──a: 123
├──child_p:
│  └──child_p: …
├──child_p:
│  └──a: 321
└──EOF
)*");
    }
}

TEST_CASE("visualization_display_width")
{
    auto visualization_width = [](const char* str, lexy::visualization_flags flags) {
        auto input   = lexy::zstring_input<lexy::utf8_encoding>(str);
        using lexeme = lexy::lexeme_for<decltype(input)>;

        return lexy::visualization_display_width(lexeme(input.data(), input.data() + input.size()),
                                                 {flags});
    };

    SUBCASE("ASCII")
    {
        auto flags = lexy::visualize_default | lexy::visualize_use_color;

        CHECK(visualization_width("abc", flags) == 3);    // abc
        CHECK(visualization_width("\n\t\\", flags) == 6); // \n\t// (but backslash)
        CHECK(visualization_width("\u1234", flags) == 6); // \u1234
    }
    SUBCASE("unicode")
    {
        auto flags = lexy::visualize_use_unicode | lexy::visualize_use_color;

        CHECK(visualization_width("abc", flags) == 3);    // abc
        CHECK(visualization_width("\n\t\\", flags) == 9); // <NL><HT>/
        CHECK(visualization_width("\u1234", flags) == 8); // <U+1234>
    }
}

