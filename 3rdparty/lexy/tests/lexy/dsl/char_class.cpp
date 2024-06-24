// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/char_class.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/code_point.hpp>
#include <lexy/dsl/unicode.hpp>

namespace
{
template <typename Rule>
void verify_empty(Rule rule, const char* name)
{
    constexpr auto callback = token_callback;

    auto utf8 = LEXY_VERIFY(lexy::utf8_encoding{});
    CHECK(utf8.status == test_result::fatal_error);
    CHECK(utf8.trace == test_trace().expected_char_class(0, name).cancel());

    auto utf16 = LEXY_VERIFY(lexy::utf16_encoding{});
    CHECK(utf16.status == test_result::fatal_error);
    CHECK(utf16.trace == test_trace().expected_char_class(0, name).cancel());

    auto utf32 = LEXY_VERIFY(lexy::utf32_encoding{});
    CHECK(utf32.status == test_result::fatal_error);
    CHECK(utf32.trace == test_trace().expected_char_class(0, name).cancel());
}

template <char32_t Cp, typename Rule>
void verify_success(Rule rule)
{
    constexpr auto cp       = lexy::code_point(Cp);
    constexpr auto callback = token_callback;
    if constexpr (cp.is_ascii())
    {
        auto result = LEXY_VERIFY(lexy::ascii_encoding{}, cp.value(), cp.value(), cp.value());
        CHECK(result.status == test_result::success);
        CHECK(result.trace == test_trace().token(doctest::toString(cp).c_str()));
    }
    else
    {
        auto utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, cp, cp, cp);
        CHECK(utf8.status == test_result::success);
        CHECK(utf8.trace == test_trace().token(doctest::toString(cp).c_str()));

        auto utf16 = LEXY_VERIFY(lexy::utf16_encoding{}, cp, cp, cp);
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().token(doctest::toString(cp).c_str()));

        auto utf32 = LEXY_VERIFY(lexy::utf32_encoding{}, cp, cp, cp);
        CHECK(utf32.status == test_result::success);
        CHECK(utf32.trace == test_trace().token(doctest::toString(cp).c_str()));
    }
}
template <char32_t... Cp, typename Rule, typename = std::enable_if_t<(sizeof...(Cp) > 1)>>
void verify_success(Rule rule)
{
    (verify_success<Cp>(rule), ...);
}

template <char32_t Cp, typename Rule>
void verify_failure(Rule rule, const char* name)
{
    constexpr auto cp       = lexy::code_point(Cp);
    constexpr auto callback = token_callback;

    if constexpr (cp.is_ascii())
    {
        auto result = LEXY_VERIFY(lexy::ascii_encoding{}, cp.value(), cp.value(), cp.value());
        CHECK(result.status == test_result::fatal_error);
        CHECK(result.trace == test_trace().expected_char_class(0, name).cancel());
    }
    else
    {
        auto utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, cp, cp, cp);
        CHECK(utf8.status == test_result::fatal_error);
        CHECK(utf8.trace == test_trace().expected_char_class(0, name).cancel());

        auto utf16 = LEXY_VERIFY(lexy::utf16_encoding{}, cp, cp, cp);
        CHECK(utf16.status == test_result::fatal_error);
        CHECK(utf16.trace == test_trace().expected_char_class(0, name).cancel());

        auto utf32 = LEXY_VERIFY(lexy::utf32_encoding{}, cp, cp, cp);
        CHECK(utf32.status == test_result::fatal_error);
        CHECK(utf32.trace == test_trace().expected_char_class(0, name).cancel());
    }
}
template <char32_t... Cp, typename Rule, typename = std::enable_if_t<(sizeof...(Cp) > 1)>>
void verify_failure(Rule rule, const char* name)
{
    (verify_failure<Cp>(rule, name), ...);
}
} // namespace

namespace
{
enum class token_kind
{
    my_kind,
};

[[maybe_unused]] constexpr const char* token_kind_name(token_kind)
{
    return "my_kind";
}
} // namespace

TEST_CASE("character class .kind and .error")
{
    struct my_error
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "my_error";
        }
    };

    SUBCASE("kind")
    {
        constexpr auto rule = dsl::ascii::alpha.kind<token_kind::my_kind>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());

        auto alpha = LEXY_VERIFY("a");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("my_kind", "a"));
    }
    SUBCASE("error")
    {
        constexpr auto rule = dsl::ascii::alpha.error<my_error>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

        auto alpha = LEXY_VERIFY("a");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("a"));
    }
    SUBCASE("kind.error")
    {
        constexpr auto rule = dsl::ascii::alpha.kind<token_kind::my_kind>.error<my_error>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

        auto alpha = LEXY_VERIFY("a");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("my_kind", "a"));
    }
    SUBCASE("error.kind")
    {
        constexpr auto rule = dsl::ascii::alpha.error<my_error>.kind<token_kind::my_kind>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

        auto alpha = LEXY_VERIFY("a");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("my_kind", "a"));
    }
}

TEST_CASE("LEXY_CHAR_CLASS")
{
    static constexpr auto rule = LEXY_CHAR_CLASS("my class", dsl::ascii::alpha);
    CHECK(lexy::is_char_class_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "my class").cancel());

    auto alpha = LEXY_VERIFY("a");
    CHECK(alpha.status == test_result::success);
    CHECK(alpha.trace == test_trace().token("a"));
}

TEST_CASE("character class alternative")
{
    SUBCASE("ASCII")
    {
        constexpr auto rule = dsl::ascii::digit / dsl::code_point.set<'a', 'b', 'c'>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "union");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>(rule);
        verify_success<'a', 'b', 'c'>(rule);

        verify_failure<'A', 'B', 'C'>(rule, "union");
        verify_failure<'x', 'y', 'z'>(rule, "union");
        verify_failure<0x00E4, 0x00DF>(rule, "union");
    }
    SUBCASE("unicode")
    {
        constexpr auto rule
            = dsl::unicode::digit / dsl::code_point.set<'a', 'b', 'c', 0x00E4, 0x00DF>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "union");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>(rule);
        verify_success<0x0660>(rule);
        verify_success<'a', 'b', 'c'>(rule);
        verify_success<0x00E4, 0x00DF>(rule);

        verify_failure<'A', 'B', 'C'>(rule, "union");
        verify_failure<'x', 'y', 'z'>(rule, "union");
        verify_failure<0x00E5, 0xAABB>(rule, "union");
    }

    SUBCASE("lit")
    {
        constexpr auto rule = dsl::ascii::digit / dsl::lit_c<'a'>;

        verify_empty(rule, "union");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>(rule);
        verify_success<'a'>(rule);

        verify_failure<'b', 'c'>(rule, "union");
        verify_failure<'x', 'y', 'z'>(rule, "union");
        verify_failure<0x00E4, 0x00DF>(rule, "union");
    }
    SUBCASE("lit_cp")
    {
        constexpr auto rule = dsl::ascii::digit / dsl::lit_cp<0x00E4>;

        verify_empty(rule, "union");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>(rule);
        verify_success<0x00E4>(rule);

        verify_failure<'a', 'b', 'c'>(rule, "union");
        verify_failure<'x', 'y', 'z'>(rule, "union");
        verify_failure<0x00E5, 0x00DF>(rule, "union");
    }

    SUBCASE("arbitrary 8-bit code points")
    {
        static constexpr auto rule
            = LEXY_CHAR_CLASS("my class", dsl::ascii::alpha / dsl::lit_b<0xE4>);
        constexpr auto callback = token_callback;

        auto default_ = LEXY_VERIFY(lexy::default_encoding{}, "\xE4");
        CHECK(default_.status == test_result::success);

        auto byte_ = LEXY_VERIFY(lexy::byte_encoding{}, 0xE4);
        CHECK(byte_.status == test_result::success);
    }

    SUBCASE("multiple literals")
    {
        constexpr auto rule = dsl::ascii::digit / dsl::lit_c<'a'> / dsl::lit_c<'b'>;

        verify_empty(rule, "union");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>(rule);
        verify_success<'a', 'b'>(rule);

        verify_failure<'c'>(rule, "union");
        verify_failure<'x', 'y', 'z'>(rule, "union");
        verify_failure<0x00E4, 0x00DF>(rule, "union");
    }
}

TEST_CASE("character class complement")
{
    SUBCASE("ASCII")
    {
        constexpr auto rule = -dsl::ascii::alpha;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "complement");

        verify_success<'0', '1', '2', '3'>(rule);
        verify_success<'.', '!', ':', '~'>(rule);
        verify_success<0x20AC, 0x222A, 0x2488>(rule);
        verify_success<0x00E4, 0x00DF, 0x06C7, 0x2160>(rule);

        verify_failure<'a', 'b', 'c'>(rule, "complement");
        verify_failure<'A', 'B', 'C'>(rule, "complement");
    }
    SUBCASE("unicode")
    {
        constexpr auto rule = -dsl::unicode::alpha;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "complement");

        verify_success<'0', '1', '2', '3'>(rule);
        verify_success<'.', '!', ':', '~'>(rule);
        verify_success<0x20AC, 0x222A, 0x2488>(rule);

        verify_failure<'a', 'b', 'c'>(rule, "complement");
        verify_failure<'A', 'B', 'C'>(rule, "complement");
        verify_failure<0x00E4, 0x00DF, 0x06C7, 0x2160>(rule, "complement");
    }

    SUBCASE("lit")
    {
        constexpr auto rule = -dsl::lit_c<'a'>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "complement");

        verify_success<'b', 'c'>(rule);
        verify_success<0x00E4, 0x20AC>(rule);

        verify_failure<'a'>(rule, "complement");
    }
    SUBCASE("lit_cp")
    {
        constexpr auto rule = -dsl::lit_cp<0x00E4>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "complement");

        verify_success<'a', 'b', 'c'>(rule);
        verify_success<0x20AC>(rule);

        verify_failure<0x00E4>(rule, "complement");
    }
}

TEST_CASE("character class minus")
{
    SUBCASE("ASCII")
    {
        constexpr auto rule = dsl::ascii::digit - dsl::code_point.set<'7', '8', '9'>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "minus");

        verify_success<'0', '1', '2', '3', '4', '5', '6'>(rule);

        verify_failure<'7', '8', '9'>(rule, "minus");
        verify_failure<'a', 'b', 'c'>(rule, "minus");
        verify_failure<0x0660, 0x0661>(rule, "minus");
    }
    SUBCASE("set unicode")
    {
        constexpr auto rule = dsl::unicode::digit - dsl::code_point.set<'7', '8', '9'>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "minus");

        verify_success<'0', '1', '2', '3', '4', '5', '6'>(rule);
        verify_success<0x0660, 0x0661>(rule);

        verify_failure<'7', '8', '9'>(rule, "minus");
        verify_failure<'a', 'b', 'c'>(rule, "minus");
    }
    SUBCASE("both unicode")
    {
        constexpr auto rule = dsl::unicode::digit - dsl::code_point.set<'7', '8', '9', 0x0661>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "minus");

        verify_success<'0', '1', '2', '3', '4', '5', '6'>(rule);
        verify_success<0x0660>(rule);

        verify_failure<'7', '8', '9'>(rule, "minus");
        verify_failure<'a', 'b', 'c'>(rule, "minus");
        verify_failure<0x0661>(rule, "minus");
    }

    SUBCASE("lit")
    {
        constexpr auto rule = dsl::ascii::digit - dsl::lit_c<'9'>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "minus");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8'>(rule);

        verify_failure<'9'>(rule, "minus");
        verify_failure<'a', 'b', 'c'>(rule, "minus");
        verify_failure<0x0660, 0x0661>(rule, "minus");
    }
    SUBCASE("lit_cp")
    {
        constexpr auto rule = dsl::unicode::digit - dsl::lit_cp<0x0661>;
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "minus");

        verify_success<'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'>(rule);
        verify_success<0x0660>(rule);

        verify_failure<'a', 'b', 'c'>(rule, "minus");
        verify_failure<0x0661>(rule, "minus");
    }

    CHECK(equivalent_rules(dsl::ascii::alpha - dsl::ascii::lower - dsl::ascii::upper,
                           dsl::ascii::alpha - (dsl::ascii::lower / dsl::ascii::upper)));
}

TEST_CASE("character class intersection")
{
    SUBCASE("ASCII")
    {
        constexpr auto rule = dsl::ascii::alpha & dsl::code_point.set<'a', 'b', 'c'>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "intersection");

        verify_success<'a', 'b', 'c'>(rule);

        verify_failure<'x', 'y', 'z'>(rule, "intersection");
        verify_failure<'A', 'B', 'C'>(rule, "intersection");
        verify_failure<0x00E4, 0x00DF>(rule, "intersection");
    }
    SUBCASE("unicode")
    {
        constexpr auto rule
            = dsl::unicode::alpha & dsl::code_point.set<'a', 'b', 'c', 0x00E4, 0x00DF>();
        CHECK(lexy::is_char_class_rule<decltype(rule)>);

        verify_empty(rule, "intersection");

        verify_success<'a', 'b', 'c'>(rule);
        verify_success<0x00E4, 0x00DF>(rule);

        verify_failure<'x', 'y', 'z'>(rule, "intersection");
        verify_failure<'A', 'B', 'C'>(rule, "intersection");
        verify_failure<0x00E5, 0xAABB>(rule, "intersection");
    }

    SUBCASE("lit")
    {
        constexpr auto rule = dsl::ascii::alpha & dsl::lit_c<'a'>;

        verify_empty(rule, "intersection");

        verify_success<'a'>(rule);

        verify_failure<'b', 'c'>(rule, "intersection");
        verify_failure<'x', 'y', 'z'>(rule, "intersection");
        verify_failure<0x00E4, 0x00DF>(rule, "intersection");
    }
    SUBCASE("lit_cp")
    {
        constexpr auto rule = dsl::unicode::alpha & dsl::lit_cp<0x00E4>;

        verify_empty(rule, "intersection");

        verify_success<0x00E4>(rule);

        verify_failure<'a', 'b', 'c'>(rule, "intersection");
        verify_failure<'x', 'y', 'z'>(rule, "intersection");
        verify_failure<0x00E5, 0x00DF>(rule, "intersection");
    }
}

