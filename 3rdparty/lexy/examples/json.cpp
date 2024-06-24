// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy/callback.hpp>     // value callbacks
#include <lexy/dsl.hpp>          // lexy::dsl::*
#include <lexy/input/file.hpp>   // lexy::read_file

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

// Datastructures for storing JSON.
// Not really the point of the example, so as simple as possible.
namespace ast
{
struct json_value;

using json_null = std::nullptr_t;
using json_bool = bool;

struct json_number
{
    std::int64_t                integer;
    std::optional<std::string>  fraction;
    std::optional<std::int16_t> exponent;
};

using json_string = std::string;

using json_array  = std::vector<json_value>;
using json_object = std::map<std::string, json_value>;

struct json_value
{
    std::variant<json_null, json_bool, json_number, json_string, json_array, json_object> v;

    template <typename T>
    json_value(T t) : v(std::move(t))
    {}

    void _indent(int level) const
    {
        for (auto i = 0; i < level; ++i)
            std::fputc(' ', stdout);
    }

    void _print(json_null, int) const
    {
        std::fputs("null", stdout);
    }
    void _print(json_bool b, int) const
    {
        if (b)
            std::fputs("true", stdout);
        else
            std::fputs("false", stdout);
    }
    void _print(const json_number& i, int) const
    {
        std::fprintf(stdout, "%" PRId64, i.integer);
        if (i.fraction)
            std::fprintf(stdout, ".%s", i.fraction->c_str());
        if (i.exponent)
            std::fprintf(stdout, "e%" PRId16, *i.exponent);
    }
    void _print(const json_string& str, int) const
    {
        std::fputc('"', stdout);
        for (auto& c : str)
            if (c == '"')
                std::fputs(R"(\")", stdout);
            else if (c == '\\')
                std::fputs(R"(\\)", stdout);
            else if (std::iscntrl(c) != 0)
                std::fprintf(stdout, "\\x%02x", static_cast<unsigned char>(c));
            else
                std::fputc(c, stdout);
        std::fputc('"', stdout);
    }
    void _print(const json_array& array, int level) const
    {
        std::fputs("[\n", stdout);

        auto first = true;
        for (auto& elem : array)
        {
            if (first)
                first = false;
            else
                std::fputs(",\n", stdout);

            _indent(level + 1);
            elem.print(level + 1);
        }

        std::fputs("\n", stdout);
        _indent(level);
        std::fputs("]", stdout);
    }
    void _print(const json_object& object, int level) const
    {
        std::fputs("{\n", stdout);

        auto first = true;
        for (auto& [key, value] : object)
        {
            if (first)
                first = false;
            else
                std::fputs(",\n", stdout);

            _indent(level + 1);
            _print(key, level + 1);
            std::fputs(" : ", stdout);
            value.print(level + 1);
        }

        std::fputs("\n", stdout);
        _indent(level);
        std::fputs("}", stdout);
    }

    void print(int level = 0) const
    {
        std::visit([&](const auto& value) { _print(value, level); }, v);
    }
};
} // namespace ast

// The grammar of JSON.
// Modelled after the specificaton of https://www.json.org.
// It is compliant modulo bugs.
namespace grammar
{
namespace dsl = lexy::dsl;

struct json_value;

// The json value null.
struct null : lexy::token_production
{
    static constexpr auto rule  = LEXY_LIT("null");
    static constexpr auto value = lexy::construct<ast::json_null>;
};

// A json value that is a boolean.
struct boolean : lexy::token_production
{
    struct true_ : lexy::transparent_production
    {
        static constexpr auto rule  = LEXY_LIT("true");
        static constexpr auto value = lexy::constant(true);
    };
    struct false_ : lexy::transparent_production
    {
        static constexpr auto rule  = LEXY_LIT("false");
        static constexpr auto value = lexy::constant(false);
    };

    static constexpr auto rule  = dsl::p<true_> | dsl::p<false_>;
    static constexpr auto value = lexy::forward<ast::json_bool>;
};

// A json value that is a number.
struct number : lexy::token_production
{
    // A signed integer parsed as int64_t.
    struct integer : lexy::transparent_production
    {
        static constexpr auto rule
            = dsl::minus_sign + dsl::integer<std::int64_t>(dsl::digits<>.no_leading_zero());
        static constexpr auto value = lexy::as_integer<std::int64_t>;
    };

    // The fractional part of a number parsed as the string.
    struct fraction : lexy::transparent_production
    {
        static constexpr auto rule  = dsl::lit_c<'.'> >> dsl::capture(dsl::digits<>);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    // The exponent of a number parsed as int64_t.
    struct exponent : lexy::transparent_production
    {
        static constexpr auto rule = [] {
            auto exp_char = dsl::lit_c<'e'> | dsl::lit_c<'E'>;
            return exp_char >> dsl::sign + dsl::integer<std::int16_t>;
        }();
        static constexpr auto value = lexy::as_integer<std::int16_t>;
    };

    static constexpr auto rule
        = dsl::peek(dsl::lit_c<'-'> / dsl::digit<>)
          >> dsl::p<integer> + dsl::opt(dsl::p<fraction>) + dsl::opt(dsl::p<exponent>);
    static constexpr auto value = lexy::construct<ast::json_number>;
};

// A json value that is a string.
struct string : lexy::token_production
{
    struct invalid_char
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "invalid character in string literal";
        }
    };

    // A mapping of the simple escape sequences to their replacement values.
    static constexpr auto escaped_symbols = lexy::symbol_table<char> //
                                                .map<'"'>('"')
                                                .map<'\\'>('\\')
                                                .map<'/'>('/')
                                                .map<'b'>('\b')
                                                .map<'f'>('\f')
                                                .map<'n'>('\n')
                                                .map<'r'>('\r')
                                                .map<'t'>('\t');

    // In JSON, a Unicode code point can be specified by its encoding in UTF-16:
    // * code points <= 0xFFFF are specified using `\uNNNN`.
    // * other code points are specified by two surrogate UTF-16 sequences.
    // However, we don't combine the two surrogates into the final code point,
    // instead keep them separate and require a later pass that merges them if necessary.
    // (This behavior is allowed by the standard).
    struct code_point_id
    {
        // We parse the integer value of a UTF-16 code unit.
        static constexpr auto rule = LEXY_LIT("u") >> dsl::code_unit_id<lexy::utf16_encoding, 4>;
        // And convert it into a code point, which might be a surrogate.
        static constexpr auto value = lexy::construct<lexy::code_point>;
    };

    static constexpr auto rule = [] {
        // Everything is allowed inside a string except for control characters.
        auto code_point = (-dsl::unicode::control).error<invalid_char>;

        // Escape sequences start with a backlash and either map one of the symbols,
        // or a Unicode code point.
        auto escape = dsl::backslash_escape.symbol<escaped_symbols>().rule(dsl::p<code_point_id>);

        // String of code_point with specified escape sequences, surrounded by ".
        // We abort string parsing if we see a newline to handle missing closing ".
        return dsl::quoted.limit(dsl::ascii::newline)(code_point, escape);
    }();

    static constexpr auto value = lexy::as_string<ast::json_string, lexy::utf8_encoding>;
};

struct unexpected_trailing_comma
{
    static constexpr auto name = "unexpected trailing comma";
};

// A json value that is an array.
struct array
{
    // A (potentially empty) list of json values, seperated by comma and surrounded by square
    // brackets.
    static constexpr auto rule
        = dsl::square_bracketed
              .opt_list(dsl::recurse<json_value>,
                        // Trailing seperators are not allowed.
                        // Use `dsl::trailing_sep()` if you want to allow it.
                        dsl::sep(dsl::comma).trailing_error<unexpected_trailing_comma>);

    static constexpr auto value = lexy::as_list<ast::json_array>;
};

// A json value that is an object.
struct object
{
    static constexpr auto rule = [] {
        // We try parsing the colon. This means that a missing colon raises an error, which is then
        // caught and parsing continues as if nothing happens. Without the try, parsing the current
        // item would be canceled immediately.
        auto item = dsl::p<string> + dsl::try_(dsl::colon) + dsl::recurse<json_value>;

        // Trailing seperators are not allowed.
        // Use `dsl::trailing_sep()` if you want to allow it.
        auto sep = dsl::sep(dsl::comma).trailing_error<unexpected_trailing_comma>;

        // A (potentially empty) list of items, seperated by comma and surrounded by curly brackets.
        return dsl::curly_bracketed.opt_list(item, sep);
    }();

    static constexpr auto value = lexy::as_collection<ast::json_object>;
};

// A json value.
struct json_value : lexy::transparent_production
{
    static constexpr auto name = "json value";

    struct expected_json_value
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "expected json value";
        }
    };

    static constexpr auto rule = [] {
        auto primitive = dsl::p<null> | dsl::p<boolean> | dsl::p<number> | dsl::p<string>;
        auto complex   = dsl::p<object> | dsl::p<array>;

        return primitive | complex | dsl::error<expected_json_value>;
    }();

    static constexpr auto value = lexy::construct<ast::json_value>;
};

// Entry point of the production.
struct json
{
    // The maximum level of nesting of JSON data structures,
    // i.e. how many `dsl::recurse` calls are allowed.
    // json.org comes with a test suite that checks for this nesting level.
    static constexpr auto max_recursion_depth = 19;

    // Whitespace is a sequence of space, tab, carriage return, or newline.
    // Add your comment syntax here.
    static constexpr auto whitespace = dsl::ascii::space / dsl::ascii::newline;

    static constexpr auto rule  = dsl::p<json_value> + dsl::eof;
    static constexpr auto value = lexy::forward<ast::json_value>;
};
} // namespace grammar

#ifndef LEXY_TEST
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: %s <filename>", argv[0]);
        return 1;
    }

    // We're requiring UTF-8 input.
    auto file = lexy::read_file<lexy::utf8_encoding>(argv[1]);
    if (!file)
    {
        std::fprintf(stderr, "file '%s' not found", argv[1]);
        return 1;
    }

    auto json = lexy::parse<grammar::json>(file.buffer(), lexy_ext::report_error);
    if (json.has_value())
        json.value().print();

    if (!json)
        return 2;
}
#endif

