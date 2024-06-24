// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/symbol.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/case_folding.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/whitespace.hpp>

TEST_CASE("symbol_table")
{
    // Note: try_parse() and key_index tested implicitly by the actual parsing code.

    SUBCASE("empty")
    {
        auto table = lexy::symbol_table<int>;
        CHECK(table.empty());
        CHECK(table.size() == 0);
        CHECK(table.begin() == table.end());
    }
    SUBCASE("non-empty")
    {
        auto table = lexy::symbol_table<int> //
                         .map<'a'>(0)
                         .map(LEXY_LIT("b"), 1)
                         .map<LEXY_SYMBOL("c")>(2)
#if LEXY_HAS_NTTP
                         .map<"abc">(3);
#else
                         .map<LEXY_SYMBOL("abc")>(3);
#endif
        CHECK(!table.empty());
        CHECK(table.size() == 4);

        auto iter = table.begin();
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("a"));
        CHECK(iter->value == 0);

        ++iter;
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("b"));
        CHECK(iter->value == 1);

        ++iter;
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("c"));
        CHECK(iter->value == 2);

        ++iter;
        CHECK(iter != table.end());
        CHECK(iter->symbol == lexy::_detail::string_view("abc"));
        CHECK(iter->value == 3);

        ++iter;
        CHECK(iter == table.end());
    }
}

namespace
{
struct my_error
{
    static constexpr auto name()
    {
        return "my error";
    }
};

struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};

constexpr auto symbols = lexy::symbol_table<int> //
                             .map<'A'>(1)
                             .map<'B'>(2)
                             .map<'C'>(3)
                             .map<LEXY_SYMBOL("Abc")>(4);

constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                              [](const char*, int value) { return value; });
} // namespace

TEST_CASE("dsl::symbol")
{
    constexpr auto symbol = lexy::dsl::symbol<symbols>;
    CHECK(lexy::is_branch_rule<decltype(symbol)>);

    SUBCASE("as rule")
    {
        constexpr auto rule = symbol;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "unknown symbol").cancel());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto B = LEXY_VERIFY("B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 2);
        CHECK(B.trace == test_trace().token("identifier", "B"));
        auto C = LEXY_VERIFY("C");
        CHECK(C.status == test_result::success);
        CHECK(C.value == 3);
        CHECK(C.trace == test_trace().token("identifier", "C"));
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 4);
        CHECK(Abc.trace == test_trace().token("identifier", "Abc"));

        auto Unknown = LEXY_VERIFY("Unknown");
        CHECK(Unknown.status == test_result::fatal_error);
        CHECK(Unknown.trace == test_trace().error(0, 0, "unknown symbol").cancel());

        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace == test_trace().token("identifier", "A"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(symbol);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto B = LEXY_VERIFY("B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 2);
        CHECK(B.trace == test_trace().token("identifier", "B"));
        auto C = LEXY_VERIFY("C");
        CHECK(C.status == test_result::success);
        CHECK(C.value == 3);
        CHECK(C.trace == test_trace().token("identifier", "C"));
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 4);
        CHECK(Abc.trace == test_trace().token("identifier", "Abc"));

        auto Unknown = LEXY_VERIFY("Unknown");
        CHECK(Unknown.status == test_result::success);
        CHECK(Unknown.value == 0);
        CHECK(Unknown.trace == test_trace());

        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace == test_trace().token("identifier", "A"));
    }

    SUBCASE(".error")
    {
        constexpr auto rule = symbol.error<my_error>;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));

        auto Unknown = LEXY_VERIFY("Unknown");
        CHECK(Unknown.status == test_result::fatal_error);
        CHECK(Unknown.trace == test_trace().error(0, 0, "my error").cancel());
    }
}

TEST_CASE("dsl::symbol(token)")
{
    constexpr auto symbol = dsl::symbol<symbols>(dsl::token(dsl::identifier(dsl::ascii::alpha)));
    CHECK(lexy::is_branch_rule<decltype(symbol)>);

    SUBCASE("as rule")
    {
        struct production : test_production_for<decltype(symbol)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "missing token").cancel());
        auto non_alpha = LEXY_VERIFY_P(production, "123");
        CHECK(non_alpha.status == test_result::fatal_error);
        CHECK(non_alpha.trace == test_trace().error(0, 0, "missing token").cancel());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("A"));
        auto B = LEXY_VERIFY_P(production, "B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 2);
        CHECK(B.trace == test_trace().token("B"));
        auto C = LEXY_VERIFY_P(production, "C");
        CHECK(C.status == test_result::success);
        CHECK(C.value == 3);
        CHECK(C.trace == test_trace().token("C"));
        auto Abc = LEXY_VERIFY_P(production, "Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 4);
        CHECK(Abc.trace == test_trace().token("Abc"));

        auto Unknown = LEXY_VERIFY_P(production, "Unknown");
        CHECK(Unknown.status == test_result::fatal_error);
        CHECK(Unknown.trace
              == test_trace().token("Unknown").error(0, 7, "unknown symbol").cancel());

        auto Ab = LEXY_VERIFY_P(production, "Ab");
        CHECK(Ab.status == test_result::fatal_error);
        CHECK(Ab.trace == test_trace().token("Ab").error(0, 2, "unknown symbol").cancel());

        auto whitespace = LEXY_VERIFY_P(production, "Abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 4);
        CHECK(whitespace.trace == test_trace().token("Abc").whitespace("..."));
    }
    SUBCASE("as branch")
    {
        struct production : test_production_for<decltype(dsl::if_(symbol))>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
        auto non_alpha = LEXY_VERIFY_P(production, "123");
        CHECK(non_alpha.status == test_result::success);
        CHECK(non_alpha.value == 0);
        CHECK(non_alpha.trace == test_trace());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("A"));
        auto B = LEXY_VERIFY_P(production, "B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 2);
        CHECK(B.trace == test_trace().token("B"));
        auto C = LEXY_VERIFY_P(production, "C");
        CHECK(C.status == test_result::success);
        CHECK(C.value == 3);
        CHECK(C.trace == test_trace().token("C"));
        auto Abc = LEXY_VERIFY_P(production, "Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 4);
        CHECK(Abc.trace == test_trace().token("Abc"));

        auto Unknown = LEXY_VERIFY_P(production, "Unknown");
        CHECK(Unknown.status == test_result::success);
        CHECK(Unknown.trace == test_trace());

        auto Ab = LEXY_VERIFY_P(production, "Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 0);
        CHECK(Ab.trace == test_trace());

        auto whitespace = LEXY_VERIFY_P(production, "Abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 4);
        CHECK(whitespace.trace == test_trace().token("Abc").whitespace("..."));
    }

    SUBCASE(".error")
    {
        struct production : test_production_for<decltype(symbol.error<my_error>)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "missing token").cancel());
        auto non_alpha = LEXY_VERIFY_P(production, "123");
        CHECK(non_alpha.status == test_result::fatal_error);
        CHECK(non_alpha.trace == test_trace().error(0, 0, "missing token").cancel());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("A"));

        auto Unknown = LEXY_VERIFY_P(production, "Unknown");
        CHECK(Unknown.status == test_result::fatal_error);
        CHECK(Unknown.trace == test_trace().token("Unknown").error(0, 7, "my error").cancel());
    }
}

TEST_CASE("dsl::symbol(identifier)")
{
    constexpr auto symbol = dsl::symbol<symbols>(dsl::identifier(dsl::ascii::alpha));
    CHECK(lexy::is_branch_rule<decltype(symbol)>);

    SUBCASE("as rule")
    {
        struct production : test_production_for<decltype(symbol)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());
        auto non_alpha = LEXY_VERIFY_P(production, "123");
        CHECK(non_alpha.status == test_result::fatal_error);
        CHECK(non_alpha.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto B = LEXY_VERIFY_P(production, "B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 2);
        CHECK(B.trace == test_trace().token("identifier", "B"));
        auto C = LEXY_VERIFY_P(production, "C");
        CHECK(C.status == test_result::success);
        CHECK(C.value == 3);
        CHECK(C.trace == test_trace().token("identifier", "C"));
        auto Abc = LEXY_VERIFY_P(production, "Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 4);
        CHECK(Abc.trace == test_trace().token("identifier", "Abc"));

        auto Unknown = LEXY_VERIFY_P(production, "Unknown");
        CHECK(Unknown.status == test_result::fatal_error);
        CHECK(
            Unknown.trace
            == test_trace().token("identifier", "Unknown").error(0, 7, "unknown symbol").cancel());

        auto Ab = LEXY_VERIFY_P(production, "Ab");
        CHECK(Ab.status == test_result::fatal_error);
        CHECK(Ab.trace
              == test_trace().token("identifier", "Ab").error(0, 2, "unknown symbol").cancel());

        auto whitespace = LEXY_VERIFY_P(production, "Abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 4);
        CHECK(whitespace.trace == test_trace().token("identifier", "Abc").whitespace("..."));
    }
    SUBCASE("as branch")
    {
        struct production : test_production_for<decltype(dsl::if_(symbol))>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
        auto non_alpha = LEXY_VERIFY_P(production, "123");
        CHECK(non_alpha.status == test_result::success);
        CHECK(non_alpha.value == 0);
        CHECK(non_alpha.trace == test_trace());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto B = LEXY_VERIFY_P(production, "B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 2);
        CHECK(B.trace == test_trace().token("identifier", "B"));
        auto C = LEXY_VERIFY_P(production, "C");
        CHECK(C.status == test_result::success);
        CHECK(C.value == 3);
        CHECK(C.trace == test_trace().token("identifier", "C"));
        auto Abc = LEXY_VERIFY_P(production, "Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 4);
        CHECK(Abc.trace == test_trace().token("identifier", "Abc"));

        auto Unknown = LEXY_VERIFY_P(production, "Unknown");
        CHECK(Unknown.status == test_result::success);
        CHECK(Unknown.trace == test_trace());

        auto Ab = LEXY_VERIFY_P(production, "Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 0);
        CHECK(Ab.trace == test_trace());

        auto whitespace = LEXY_VERIFY_P(production, "Abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 4);
        CHECK(whitespace.trace == test_trace().token("identifier", "Abc").whitespace("..."));
    }

    SUBCASE(".error")
    {
        struct production : test_production_for<decltype(symbol.error<my_error>)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());
        auto non_alpha = LEXY_VERIFY_P(production, "123");
        CHECK(non_alpha.status == test_result::fatal_error);
        CHECK(non_alpha.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));

        auto Unknown = LEXY_VERIFY_P(production, "Unknown");
        CHECK(Unknown.status == test_result::fatal_error);
        CHECK(Unknown.trace
              == test_trace().token("identifier", "Unknown").error(0, 7, "my error").cancel());
    }
}

namespace
{
constexpr auto symbols_case_folded = lexy::symbol_table<int>.case_folding(dsl::ascii::case_folding)
                             .map<'a'>(1)
                             .map<'b'>(2)
                             .map<'c'>(3)
                             .map<LEXY_SYMBOL("abc")>(4);
}

TEST_CASE("dsl::symbol with case folding")
{
    constexpr auto rule = lexy::dsl::symbol<symbols_case_folded>;
    CHECK(lexy::is_branch_rule<decltype(rule)>);

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "unknown symbol").cancel());

    auto A = LEXY_VERIFY("A");
    CHECK(A.status == test_result::success);
    CHECK(A.value == 1);
    CHECK(A.trace == test_trace().token("identifier", "A"));
    auto B = LEXY_VERIFY("B");
    CHECK(B.status == test_result::success);
    CHECK(B.value == 2);
    CHECK(B.trace == test_trace().token("identifier", "B"));
    auto C = LEXY_VERIFY("C");
    CHECK(C.status == test_result::success);
    CHECK(C.value == 3);
    CHECK(C.trace == test_trace().token("identifier", "C"));
    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.value == 4);
    CHECK(abc.trace == test_trace().token("identifier", "abc"));
    auto Abc = LEXY_VERIFY("Abc");
    CHECK(Abc.status == test_result::success);
    CHECK(Abc.value == 4);
    CHECK(Abc.trace == test_trace().token("identifier", "Abc"));

    auto Unknown = LEXY_VERIFY("Unknown");
    CHECK(Unknown.status == test_result::fatal_error);
    CHECK(Unknown.trace == test_trace().error(0, 0, "unknown symbol").cancel());

    auto Ab = LEXY_VERIFY("Ab");
    CHECK(Ab.status == test_result::success);
    CHECK(Ab.value == 1);
    CHECK(Ab.trace == test_trace().token("identifier", "A"));
}

