// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/parse.hpp>

#include <doctest/doctest.h>
#include <lexy/callback.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/brackets.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/punctuator.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/input/string_input.hpp>
#include <vector>

namespace parse_value
{
//=== AST ===//
struct string_pair
{
    lexy::_detail::string_view a;
    lexy::_detail::string_view b;
};

//=== grammar ===//
namespace dsl = lexy::dsl;

struct string_p
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alnum);

    static constexpr auto value = lexy::as_string<lexy::_detail::string_view>;
};

struct string_pair_p
{
    static constexpr auto rule
        = dsl::parenthesized(dsl::p<string_p> + dsl::comma + dsl::p<string_p>);

    static constexpr auto value = lexy::construct<string_pair>;
};

using prod = string_pair_p;
} // namespace parse_value

namespace parse_sink
{
namespace dsl = lexy::dsl;

using parse_value::string_p;

struct string_list_p
{
    static constexpr auto rule = dsl::parenthesized.opt_list(dsl::p<string_p>, sep(dsl::comma));

    static constexpr auto value = lexy::as_list<std::vector<lexy::_detail::string_view>>;
};

using prod = string_list_p;
} // namespace parse_sink

namespace parse_sink_cb
{
namespace dsl = lexy::dsl;

using parse_value::string_p;

struct string_list_p
{
    static constexpr auto rule = dsl::parenthesized.opt_list(dsl::p<string_p>, sep(dsl::comma));

    static constexpr auto value = [] {
        auto sink = lexy::as_list<std::vector<lexy::_detail::string_view>>;
        auto cb
            = lexy::callback<std::size_t>([](lexy::nullopt) { return 0u; },
                                          [](const std::vector<lexy::_detail::string_view>& vec) {
                                              return vec.size();
                                          });

        return sink >> cb;
    }();
};

using prod = string_list_p;
} // namespace parse_sink_cb

namespace parse_void
{
namespace dsl = lexy::dsl;

using parse_value::string_p;

struct string_pair_p
{
    static constexpr auto rule
        = dsl::parenthesized(dsl::p<string_p> + dsl::comma + dsl::p<string_p>);
    static constexpr auto value = lexy::noop;
};

using prod = string_pair_p;
} // namespace parse_void

TEST_CASE("parse")
{
    SUBCASE("value")
    {
        using namespace parse_value;

        constexpr auto empty = lexy::parse<prod>(lexy::zstring_input(""), lexy::noop);
        CHECK(!empty);

        constexpr auto abc_abc = lexy::parse<prod>(lexy::zstring_input("(abc,abc)"), lexy::noop);
        CHECK(abc_abc);
        CHECK(abc_abc.value().a == "abc");
        CHECK(abc_abc.value().b == "abc");

        constexpr auto abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,123)"), lexy::noop);
        CHECK(abc_123);
        CHECK(abc_123.value().a == "abc");
        CHECK(abc_123.value().b == "123");
    }
    SUBCASE("sink")
    {
        using namespace parse_sink;

        auto empty = lexy::parse<prod>(lexy::zstring_input(""), lexy::noop);
        CHECK(!empty);

        auto parens = lexy::parse<prod>(lexy::zstring_input("()"), lexy::noop);
        CHECK(parens);
        INFO(parens.value().at(0).size());
        CHECK(parens.value().empty());

        auto abc = lexy::parse<prod>(lexy::zstring_input("(abc)"), lexy::noop);
        CHECK(abc);
        CHECK(abc.value().size() == 1);
        CHECK(abc.value().at(0) == "abc");

        auto abc_abc = lexy::parse<prod>(lexy::zstring_input("(abc,abc)"), lexy::noop);
        CHECK(abc_abc);
        CHECK(abc_abc.value().size() == 2);
        CHECK(abc_abc.value().at(0) == "abc");
        CHECK(abc_abc.value().at(1) == "abc");

        auto abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,123)"), lexy::noop);
        CHECK(abc_123);
        CHECK(abc_123.value().size() == 2);
        CHECK(abc_123.value().at(0) == "abc");
        CHECK(abc_123.value().at(1) == "123");

        auto abc_abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,abc,123)"), lexy::noop);
        CHECK(abc_abc_123);
        CHECK(abc_abc_123.value().size() == 3);
        CHECK(abc_abc_123.value().at(0) == "abc");
        CHECK(abc_abc_123.value().at(1) == "abc");
        CHECK(abc_abc_123.value().at(2) == "123");
    }
    SUBCASE("sink_cb")
    {
        using namespace parse_sink_cb;

        auto empty = lexy::parse<prod>(lexy::zstring_input(""), lexy::noop);
        CHECK(!empty);

        auto parens = lexy::parse<prod>(lexy::zstring_input("()"), lexy::noop);
        CHECK(parens);
        CHECK(parens.value() == 0);

        auto abc = lexy::parse<prod>(lexy::zstring_input("(abc)"), lexy::noop);
        CHECK(abc);
        CHECK(abc.value() == 1);

        auto abc_abc = lexy::parse<prod>(lexy::zstring_input("(abc,abc)"), lexy::noop);
        CHECK(abc_abc);
        CHECK(abc_abc.value() == 2);

        auto abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,123)"), lexy::noop);
        CHECK(abc_123);
        CHECK(abc_123.value() == 2);

        auto abc_abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,abc,123)"), lexy::noop);
        CHECK(abc_abc_123);
        CHECK(abc_abc_123.value() == 3);
    }
    SUBCASE("void")
    {
        using namespace parse_void;

        constexpr auto empty = lexy::parse<prod>(lexy::zstring_input(""), lexy::noop);
        CHECK(!empty);

        constexpr auto abc_abc = lexy::parse<prod>(lexy::zstring_input("(abc,abc)"), lexy::noop);
        CHECK(abc_abc);

        constexpr auto abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,123)"), lexy::noop);
        CHECK(abc_123);
    }
}

namespace parse_value_state
{
namespace dsl = lexy::dsl;

struct string_pair
{
    int                        state;
    lexy::_detail::string_view a;
    lexy::_detail::string_view b;
};

using parse_value::string_p;

struct string_pair_p
{
    static constexpr auto rule
        = dsl::parenthesized(dsl::p<string_p> + dsl::comma + dsl::p<string_p>);

    static constexpr auto value
        = lexy::bind(lexy::construct<string_pair>, lexy::parse_state, lexy::values);
};

using prod = string_pair_p;
} // namespace parse_value_state

TEST_CASE("parse with state")
{
    SUBCASE("value")
    {
        using namespace parse_value_state;

        constexpr auto empty = lexy::parse<prod>(lexy::zstring_input(""), 42, lexy::noop);
        CHECK(!empty);

        constexpr auto abc_abc
            = lexy::parse<prod>(lexy::zstring_input("(abc,abc)"), 42, lexy::noop);
        CHECK(abc_abc);
        CHECK(abc_abc.value().state == 42);
        CHECK(abc_abc.value().a == "abc");
        CHECK(abc_abc.value().b == "abc");

        constexpr auto abc_123
            = lexy::parse<prod>(lexy::zstring_input("(abc,123)"), 42, lexy::noop);
        CHECK(abc_123);
        CHECK(abc_abc.value().state == 42);
        CHECK(abc_123.value().a == "abc");
        CHECK(abc_123.value().b == "123");
    }
}

