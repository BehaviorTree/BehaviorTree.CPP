// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/delimited.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/if.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};

struct delim_sink
{
    std::size_t result;

    using return_type = std::size_t;

    template <typename... Args>
    void operator()(const Args&...)
    {
        ++result;
    }
    template <typename Reader>
    void operator()(lexy::lexeme<Reader> lex)
    {
        result += lex.size();
    }

    std::size_t finish() &&
    {
        return result;
    }
};

template <typename Callback>
struct delim_callback : Callback
{
    constexpr delim_callback(Callback cb) : Callback(cb) {}

    auto sink() const
    {
        return delim_sink{0};
    }
};
} // namespace

TEST_CASE("dsl::delimited(open, close)")
{
    constexpr auto delimited
        = dsl::delimited(dsl::capture(dsl::lit_c<'('>), dsl::capture(dsl::lit_c<')'>));
    CHECK(equivalent_rules(delimited.open(), dsl::capture(dsl::lit_c<'('>)));
    CHECK(equivalent_rules(delimited.close(), dsl::capture(dsl::lit_c<')'>)));

    constexpr delim_callback callback
        = lexy::callback<int>([](const char*) { return -11; },
                              [](const char* begin, lexy::string_lexeme<> open, std::size_t count,
                                 lexy::string_lexeme<> close) {
                                  CHECK(open.begin() == begin);
                                  CHECK(open.size() == 1);
                                  CHECK(open[0] == '(');

                                  CHECK(close.begin() >= begin + 1 + count);
                                  CHECK(close.size() == 1);
                                  CHECK(close[0] == ')');

                                  return int(count);
                              });

    SUBCASE("as rule")
    {
        constexpr auto rule = delimited(dsl::ascii::character);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "(", 0).cancel());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto one = LEXY_VERIFY("(a)");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("(").token("a").literal(")"));
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == test_trace().literal("(").token("ab").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto invalid = LEXY_VERIFY("(a\x80-c)");
        CHECK(invalid.status == test_result::recovered_error);
        CHECK(invalid.value == 3);
        CHECK(invalid.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .token("-c")
                     .literal(")"));
        auto invalid_end = LEXY_VERIFY("(a\x80)");
        CHECK(invalid_end.status == test_result::recovered_error);
        CHECK(invalid_end.value == 1);
        CHECK(invalid_end.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .literal(")"));

        auto unterminated = LEXY_VERIFY("(ab");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto whitespace = LEXY_VERIFY_P(production, "(.abc.).");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 5);
        CHECK(whitespace.trace
              == test_trace().literal("(").token(".abc.").literal(")").whitespace("."));
    }
    SUBCASE("as rule with custom char class")
    {
        constexpr auto class_
            = LEXY_CHAR_CLASS("my class", dsl::ascii::alpha / dsl::ascii::punct / dsl::lit_b<0xE4>);
        constexpr auto rule = delimited(class_);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "(", 0).cancel());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto one = LEXY_VERIFY("(a)");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("(").token("a").literal(")"));
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == test_trace().literal("(").token("ab").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto custom = LEXY_VERIFY("(a\xE4-c)");
        CHECK(custom.status == test_result::success);
        CHECK(custom.value == 4);
        CHECK(custom.trace == test_trace().literal("(").token("a\\xE4-c").literal(")"));

        auto custom_end = LEXY_VERIFY("(a\xE4)");
        CHECK(custom_end.status == test_result::success);
        CHECK(custom_end.value == 2);
        CHECK(custom_end.trace == test_trace().literal("(").token("a\\xE4").literal(")"));

        auto unterminated = LEXY_VERIFY("(ab");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto whitespace = LEXY_VERIFY_P(production, "(.abc.).");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 5);
        CHECK(whitespace.trace
              == test_trace().literal("(").token(".abc.").literal(")").whitespace("."));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(delimited(dsl::ascii::character));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto invalid = LEXY_VERIFY("(a\x80-c)");
        CHECK(invalid.status == test_result::recovered_error);
        CHECK(invalid.value == 3);
        CHECK(invalid.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .token("-c")
                     .literal(")"));
        auto invalid_end = LEXY_VERIFY("(a\x80)");
        CHECK(invalid_end.status == test_result::recovered_error);
        CHECK(invalid_end.value == 1);
        CHECK(invalid_end.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .literal(")"));

        auto unterminated = LEXY_VERIFY("(ab");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto whitespace = LEXY_VERIFY_P(production, "(.abc.).");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 5);
        CHECK(whitespace.trace
              == test_trace().literal("(").token(".abc.").literal(")").whitespace("."));
    }

    SUBCASE("with escape")
    {
        constexpr auto escape = dsl::dollar_escape.rule(dsl::lit_c<'a'>).rule(dsl::lit_c<')'>);
        constexpr auto rule   = delimited(dsl::ascii::character, escape);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "(", 0).cancel());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto invalid = LEXY_VERIFY("(a\x80-c)");
        CHECK(invalid.status == test_result::recovered_error);
        CHECK(invalid.value == 3);
        CHECK(invalid.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .token("-c")
                     .literal(")"));

        auto unterminated = LEXY_VERIFY("(ab");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());

        auto escape_a = LEXY_VERIFY("(a$ab)");
        CHECK(escape_a.status == test_result::success);
        CHECK(escape_a.value == 2);
        CHECK(escape_a.trace
              == test_trace().literal("(").token("a").literal("$").literal("a").token("b").literal(
                  ")"));
        auto escape_close = LEXY_VERIFY("(a$)b)");
        CHECK(escape_close.status == test_result::success);
        CHECK(escape_close.value == 2);
        CHECK(escape_close.trace
              == test_trace().literal("(").token("a").literal("$").literal(")").token("b").literal(
                  ")"));
        auto escape_unknown = LEXY_VERIFY("(a$?b)");
        CHECK(escape_unknown.status == test_result::recovered_error);
        CHECK(escape_unknown.value == 3);
        CHECK(escape_unknown.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .literal("$")
                     .error(2, 3, "invalid escape sequence")
                     .token("?b")
                     .literal(")"));
        auto escape_escape = LEXY_VERIFY("(a$$ab)");
        CHECK(escape_escape.status == test_result::recovered_error);
        CHECK(escape_escape.value == 2);
        CHECK(escape_escape.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .literal("$")
                     .error(2, 3, "invalid escape sequence")
                     .literal("$")
                     .literal("a")
                     .token("b")
                     .literal(")"));
    }
    SUBCASE(".limit()")
    {
        constexpr auto rule = delimited.limit(LEXY_ASCII_ONE_OF("\n!"))(dsl::ascii::character);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "(", 0).cancel());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto invalid = LEXY_VERIFY("(a\x80-c)");
        CHECK(invalid.status == test_result::recovered_error);
        CHECK(invalid.value == 3);
        CHECK(invalid.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .token("-c")
                     .literal(")"));
        auto invalid_limit = LEXY_VERIFY("(a\x80\nc)");
        CHECK(invalid_limit.status == test_result::fatal_error);
        CHECK(invalid_limit.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .error(1, 3, "missing delimiter")
                     .cancel());

        auto unterminated = LEXY_VERIFY("(ab");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());

        auto unterminated_nl = LEXY_VERIFY("(ab\nc)");
        CHECK(unterminated_nl.status == test_result::fatal_error);
        CHECK(unterminated_nl.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());
        auto unterminated_mark = LEXY_VERIFY("(ab!c)");
        CHECK(unterminated_mark.status == test_result::fatal_error);
        CHECK(unterminated_mark.trace
              == test_trace().literal("(").token("ab").error(1, 3, "missing delimiter").cancel());
    }
    SUBCASE(".limit() with error")
    {
        struct tag
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "error";
            }
        };

        constexpr auto rule = delimited.limit<tag>(LEXY_ASCII_ONE_OF("\n!"))(dsl::ascii::character);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "(", 0).cancel());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto invalid = LEXY_VERIFY("(a\x80-c)");
        CHECK(invalid.status == test_result::recovered_error);
        CHECK(invalid.value == 3);
        CHECK(invalid.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .token("-c")
                     .literal(")"));
        auto invalid_limit = LEXY_VERIFY("(a\x80\nc)");
        CHECK(invalid_limit.status == test_result::fatal_error);
        CHECK(invalid_limit.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "ASCII")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .error(1, 3, "error")
                     .cancel());

        auto unterminated = LEXY_VERIFY("(ab");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("(").token("ab").error(1, 3, "error").cancel());

        auto unterminated_nl = LEXY_VERIFY("(ab\nc)");
        CHECK(unterminated_nl.status == test_result::fatal_error);
        CHECK(unterminated_nl.trace
              == test_trace().literal("(").token("ab").error(1, 3, "error").cancel());
        auto unterminated_mark = LEXY_VERIFY("(ab!c)");
        CHECK(unterminated_mark.status == test_result::fatal_error);
        CHECK(unterminated_mark.trace
              == test_trace().literal("(").token("ab").error(1, 3, "error").cancel());
    }
    SUBCASE("minus")
    {
        constexpr auto rule = delimited(dsl::ascii::character - LEXY_ASCII_ONE_OF("X"));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "(", 0).cancel());

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("(").literal(")"));
        auto one = LEXY_VERIFY("(a)");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("(").token("a").literal(")"));
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == test_trace().literal("(").token("ab").literal(")"));
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == test_trace().literal("(").token("abc").literal(")"));

        auto invalid = LEXY_VERIFY("(a\x80-c)");
        CHECK(invalid.status == test_result::recovered_error);
        CHECK(invalid.value == 3);
        CHECK(invalid.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "minus")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .token("-c")
                     .literal(")"));
        auto invalid_end = LEXY_VERIFY("(a\x80)");
        CHECK(invalid_end.status == test_result::recovered_error);
        CHECK(invalid_end.value == 1);
        CHECK(invalid_end.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "minus")
                     .recovery()
                     .error_token("\\x80")
                     .finish()
                     .literal(")"));

        auto minus = LEXY_VERIFY("(aXc)");
        CHECK(minus.status == test_result::recovered_error);
        CHECK(minus.value == 2);
        CHECK(minus.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "minus")
                     .recovery()
                     .error_token("X")
                     .finish()
                     .token("c")
                     .literal(")"));
        auto minus_end = LEXY_VERIFY("(aX)");
        CHECK(minus_end.status == test_result::recovered_error);
        CHECK(minus_end.value == 1);
        CHECK(minus_end.trace
              == test_trace()
                     .literal("(")
                     .token("a")
                     .expected_char_class(2, "minus")
                     .recovery()
                     .error_token("X")
                     .finish()
                     .literal(")"));
    }
}

TEST_CASE("dsl::delimited(delim)")
{
    CHECK(equivalent_rules(dsl::delimited(dsl::lit_c<'"'>),
                           dsl::delimited(dsl::lit_c<'"'>, dsl::lit_c<'"'>)));

    CHECK(equivalent_rules(dsl::quoted, dsl::delimited(dsl::lit_c<'"'>)));
    CHECK(equivalent_rules(dsl::single_quoted, dsl::delimited(dsl::lit_c<'\''>)));

    CHECK(equivalent_rules(dsl::triple_quoted, dsl::delimited(LEXY_LIT("\"\"\""))));

    CHECK(equivalent_rules(dsl::backticked, dsl::delimited(dsl::lit_c<'`'>)));
    CHECK(equivalent_rules(dsl::double_backticked, dsl::delimited(LEXY_LIT("``"))));
    CHECK(equivalent_rules(dsl::triple_backticked, dsl::delimited(LEXY_LIT("```"))));
}

namespace
{
constexpr auto symbols = lexy::symbol_table<int>;
}

TEST_CASE("dsl::escape")
{
    constexpr auto escape = dsl::escape(dsl::lit_c<'$'>);
    CHECK(equivalent_rules(escape.capture(LEXY_LIT("abc")),
                           escape.rule(dsl::capture(LEXY_LIT("abc")))));
    CHECK(equivalent_rules(escape.symbol<symbols>(dsl::ascii::character),
                           escape.rule(dsl::symbol<symbols>(dsl::ascii::character))));
    CHECK(equivalent_rules(escape.symbol<symbols>(), escape.rule(dsl::symbol<symbols>)));

    CHECK(equivalent_rules(dsl::backslash_escape, dsl::escape(dsl::lit_c<'\\'>)));
    CHECK(equivalent_rules(dsl::dollar_escape, escape));
}

