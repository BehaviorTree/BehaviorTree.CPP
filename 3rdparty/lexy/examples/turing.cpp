// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

// This example demonstrates that lexy is turing complete:
// it parses and executes (!) a simple programming language by abusing the context variables.
// Don't use these techniques in your real lexy programs.
// Read the corresponding blog post here: https://foonathan.net/2021/11/lexy-turing/

#include <cstdio>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>

// We need to cheat a little bit and add a custom rule that parses a rule and rewinds the input.
// This is the same behavior as `dsl::peek()`, but `dsl::peek()` does not give access to the context
// variables, so we need to roll our own.
namespace dsl_ext
{
template <typename Rule>
struct _rewind : lexy::dsl::rule_base
{
    template <typename NextParser>
    struct p
    {
        struct continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                               typename Reader::iterator saved_pos, Args&&... args)
            {
                reader.set_position(saved_pos);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto saved_pos = reader.position();
            return lexy::parser_for<Rule, continuation>::parse(context, reader, saved_pos,
                                                               LEXY_FWD(args)...);
        }
    };
};

template <typename Rule>
constexpr auto rewind(Rule)
{
    return _rewind<Rule>{};
}
} // namespace dsl_ext

// The grammar for our language.
// It consists of unsigned integer variables that can be assigned plus if and while statements.
namespace grammar
{
namespace dsl = lexy::dsl;

template <char... Vars>
struct statement;

//=== tokens ===//
constexpr auto comment = LEXY_LIT("//") >> dsl::until(dsl::ascii::newline);

constexpr auto identifier = dsl::identifier(dsl::ascii::alpha);
constexpr auto kw_if      = LEXY_KEYWORD("if", identifier);
constexpr auto kw_else    = LEXY_KEYWORD("else", identifier);
constexpr auto kw_while   = LEXY_KEYWORD("while", identifier);

// Number literal in base 1, i.e. tally marks.
struct number : lexy::token_production
{
    static constexpr auto rule  = dsl::while_(dsl::lit_c<'|'>);
    static constexpr auto value = lexy::forward<void>;
};

//=== variables ===//
template <char Name>
struct var_id
{};

// A variable is an unsigned integer.
// (dsl::context_counter can be negative, but we can't implement assignment otherwise.)
template <char Name>
constexpr auto var = dsl::context_counter<var_id<Name>>;

struct unknown_variable
{
    static constexpr auto name = "unknown variable";
};
constexpr auto unknown_variable_error = dsl::error<unknown_variable>(identifier);

//=== statements ===//
// Variable assignment/increment/decrement.
template <char Name>
struct var_stmt
{
    static constexpr auto name = "var-stmt";

    // Var := number | Var += number | Var -= number
    static constexpr auto rule = [] {
        // There is no rule to reset a counter, so we need to decrement it as necessary.
        // (This requires non-negative values).
        auto reset = dsl::loop(var<Name>.is_zero() >> dsl::break_ | dsl::else_ >> var<Name>.dec());

        // Once it's zero, we add the number.
        auto assign = LEXY_LIT(":=") >> reset + var<Name>.push(dsl::p<number>);

        auto add = LEXY_LIT("+=") >> var<Name>.push(dsl::p<number>);
        auto dec = LEXY_LIT("-=") >> var<Name>.pop(dsl::p<number>);

        return dsl::keyword<Name>(identifier) >> (assign | add | dec) + dsl::semicolon;
    }();

    static constexpr auto value = lexy::noop;
};

// Executes a body of an if/while.
template <char... Vars>
struct execute_body
{
    static constexpr auto name = "execute-body";

    // Parsing the body executes it.
    static constexpr auto rule  = dsl::curly_bracketed.opt_list(dsl::recurse<statement<Vars...>>);
    static constexpr auto value = lexy::forward<void>;
};

// Skips until we find the next curly brackets, keeping track of the opening brackets.
struct skip_body
{
    static constexpr auto name = "skip-body";

    static constexpr auto rule = [] {
        auto bracket_counter = dsl::context_counter<skip_body>;
        auto check_balance   = dsl::if_(bracket_counter.is_zero() >> dsl::break_);

        auto open  = LEXY_LIT("{") >> bracket_counter.inc();
        auto close = LEXY_LIT("}") >> bracket_counter.dec();
        auto skip  = open | close | comment | dsl::ascii::character;

        return bracket_counter.create() + dsl::loop(skip + check_balance);
    }();

    static constexpr auto value = lexy::forward<void>;
};

// Checks whether a variable is non-zero.
// if x { statments } else { statements }
// else is optional
template <char... Vars>
struct if_stmt
{
    static constexpr auto name = "if-stmt";

    template <char Var>
    struct impl
    {
        static constexpr auto name = "if-stmt-impl";

        static constexpr auto rule = [] {
            auto non_zero = dsl::p<execute_body<Vars...>> + dsl::if_(kw_else >> dsl::p<skip_body>);
            auto zero     = dsl::p<skip_body> + dsl::if_(kw_else >> dsl::p<execute_body<Vars...>>);

            // If the variable is zero, we skip, otherwise, we execute.
            auto select = var<Var>.is_zero() >> zero | dsl::else_ >> non_zero;

            return dsl::lit_c<Var> >> select;
        }();

        static constexpr auto value = lexy::forward<void>;
    };

    static constexpr auto rule  = kw_if >> (dsl::p<impl<Vars>> | ... | unknown_variable_error);
    static constexpr auto value = lexy::forward<void>;
};

// Loops while a variable is non-zero.
// while x { statements }
template <char... Vars>
struct while_stmt
{
    static constexpr auto name = "while-stmt";

    template <char Var>
    struct impl
    {
        static constexpr auto name = "while-stmt-impl";

        static constexpr auto rule = [] {
            // After executing the body, rewind back.
            auto non_zero = dsl_ext::rewind(dsl::p<execute_body<Vars...>>);
            // After skipping the body, we break.
            auto zero = dsl::p<skip_body> + dsl::break_;

            // We repeatedly select execution or skipping.
            auto loop = dsl::loop(var<Var>.is_zero() >> zero | dsl::else_ >> non_zero);

            return dsl::lit_c<Var> >> loop;
        }();

        static constexpr auto value = lexy::forward<void>;
    };

    static constexpr auto rule  = kw_while >> (dsl::p<impl<Vars>> | ... | unknown_variable_error);
    static constexpr auto value = lexy::forward<void>;
};

template <char... Vars>
struct statement
{
    static constexpr auto name = "statement";

    static constexpr auto rule = [] {
        auto if_stmts    = dsl::p<if_stmt<Vars...>>;
        auto while_stmts = dsl::p<while_stmt<Vars...>>;
        auto var_stmts   = (dsl::p<var_stmt<Vars>> | ... | unknown_variable_error);

        return if_stmts | while_stmts | dsl::else_ >> var_stmts;
    }();

    static constexpr auto value = lexy::forward<void>;
};

//=== program ===//
// A program is a list of statements.
template <char... Vars>
struct program
{
    static constexpr auto name = "program";

    static constexpr auto whitespace = dsl::ascii::space | comment;

    static constexpr auto rule = [] {
        // Create all variables (initialized to zero).
        auto create = (var<Vars>.create() + ...);

        // Parse (and thus execute) a list of statements.
        auto run = dsl::loop(dsl::eof >> dsl::break_ | dsl::else_ >> dsl::p<statement<Vars...>>);

        // We produce the value of o (for output) as the parse result.
        auto output = var<'o'>.value();

        return create + run + output;
    }();

    static constexpr auto value = lexy::construct<int>;
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

    auto file = lexy::read_file(argv[1]);
    if (!file)
    {
        std::fprintf(stderr, "file '%s' not found", argv[1]);
        return 1;
    }

    // Note: this instantiates a lot of code and thus it takes a while to compile.
    // (It is not representative of an actual lexy grammar).
    using program
        = grammar::program<'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                           'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'>;
    auto result = lexy::parse<program>(file.buffer(), lexy_ext::report_error);
    if (!result)
        return 2;

    std::printf("result: %d\n", result.value());
}
#endif

