// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <map>
#include <memory>
#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/shell.hpp>

// A shell with a couple of basic commands.
namespace shell
{
struct interpreter
{
    // Manages I/O.
    lexy_ext::shell<lexy_ext::default_prompt<lexy::utf8_encoding>> shell;
    // The values of variables.
    std::map<std::string, std::string> variables;

    // Retrieves the value of a variable.
    const std::string& lookup_var(const std::string& name) const
    {
        auto iter = variables.find(name);
        if (iter == variables.end())
        {
            static const std::string not_found;
            return not_found;
        }

        return iter->second;
    }
};

// Special directives that control what happens with entered commands.
enum class directive
{
    // Command following it is executed (the default).
    execute,
    // Parsing of the command following it is traced.
    trace,
    // All variables are printed.
    vars,
};

class cmd_base
{
public:
    virtual ~cmd_base() = default;

    // Returns true if the shell should exit.
    virtual bool execute(interpreter& intp) const = 0;
};
using command = std::unique_ptr<cmd_base>;

// Exits the shell.
class cmd_exit : public cmd_base
{
public:
    bool execute(interpreter& intp) const override
    {
        intp.shell.write_message()("Goodbye.");
        return true;
    }
};

// Prints output.
class cmd_echo : public cmd_base
{
public:
    explicit cmd_echo(std::string msg) : _msg(LEXY_MOV(msg)) {}

    bool execute(interpreter& intp) const override
    {
        intp.shell.write_message()(_msg.data(), _msg.size());
        return false;
    }

private:
    std::string _msg;
};

// Sets the value of a variable.
class cmd_set : public cmd_base
{
public:
    explicit cmd_set(std::string name, std::string value)
    : _name(LEXY_MOV(name)), _value(LEXY_MOV(value))
    {}

    bool execute(interpreter& intp) const override
    {
        intp.variables[_name] = _value;
        return false;
    }

private:
    std::string _name;
    std::string _value;
};
} // namespace shell

namespace grammar
{
namespace dsl = lexy::dsl;

// A bare argument or command name.
constexpr auto identifier = dsl::identifier(dsl::ascii::alnum);

//=== The arguments ===//
struct invalid_str_char
{
    static constexpr auto name = "invalid string character";
};

// The content of a string literal: any unicode code point except for control characters.
constexpr auto str_char = (-dsl::unicode::control).error<invalid_str_char>;

// An unquoted sequence of characters.
struct arg_bare
{
    static constexpr auto rule  = identifier;
    static constexpr auto value = lexy::as_string<std::string>;
};

// A string without escape characters.
struct arg_string
{
    static constexpr auto rule  = dsl::single_quoted(str_char);
    static constexpr auto value = lexy::as_string<std::string>;
};

// A string with escape characters.
struct arg_quoted
{
    struct interpolation
    {
        static constexpr auto rule = dsl::curly_bracketed(dsl::recurse<struct argument>);
        static constexpr auto value
            = lexy::bind(lexy::callback<std::string>(&shell::interpreter::lookup_var),
                         lexy::parse_state, lexy::values);
    };

    static constexpr auto escaped_sequences = lexy::symbol_table<char> //
                                                  .map<'"'>('"')
                                                  .map<'\\'>('\\')
                                                  .map<'/'>('/')
                                                  .map<'b'>('\b')
                                                  .map<'f'>('\f')
                                                  .map<'n'>('\n')
                                                  .map<'r'>('\r')
                                                  .map<'t'>('\t');

    static constexpr auto rule = [] {
        // C style escape sequences.
        auto backslash_escape = dsl::backslash_escape.symbol<escaped_sequences>();
        // Interpolation.
        auto dollar_escape = dsl::dollar_escape.rule(dsl::p<interpolation>);

        return dsl::quoted.limit(dsl::ascii::newline)(str_char, backslash_escape, dollar_escape);
    }();
    static constexpr auto value = lexy::as_string<std::string>;
};

// An argument that expands to the value of a variable.
struct arg_var
{
    static constexpr auto rule = [] {
        auto bare      = dsl::p<arg_bare>;
        auto bracketed = dsl::curly_bracketed(dsl::recurse<argument>);
        auto name      = bracketed | dsl::else_ >> bare;

        return dsl::dollar_sign >> name;
    }();
    static constexpr auto value
        = lexy::bind(lexy::callback<std::string>(&shell::interpreter::lookup_var),
                     lexy::parse_state, lexy::values);
};

// An argument to a command.
struct argument
{
    struct invalid
    {
        static constexpr auto name = "invalid argument character";
    };

    static constexpr auto rule = dsl::p<arg_string> | dsl::p<arg_quoted> //
                                 | dsl::p<arg_var> | dsl::p<arg_bare>    //
                                 | dsl::error<invalid>;
    static constexpr auto value = lexy::forward<std::string>;
};

// The separator between arguments.
constexpr auto arg_sep = [] {
    struct missing_argument
    {
        static constexpr auto name()
        {
            return "missing argument";
        }
    };

    auto blank      = dsl::ascii::blank;
    auto escaped_nl = dsl::backslash >> dsl::newline;
    auto sep        = dsl::must(blank | escaped_nl).error<missing_argument>;
    return dsl::while_one(sep);
}();

//=== the commands ===//
struct cmd_exit
{
    static constexpr auto rule  = LEXY_KEYWORD("exit", identifier) | dsl::eof;
    static constexpr auto value = lexy::new_<shell::cmd_exit, shell::command>;
};

struct cmd_echo
{
    static constexpr auto rule  = LEXY_KEYWORD("echo", identifier) >> arg_sep + dsl::p<argument>;
    static constexpr auto value = lexy::new_<shell::cmd_echo, shell::command>;
};

struct cmd_set
{
    static constexpr auto rule = LEXY_KEYWORD("set", identifier)
                                 >> arg_sep + dsl::p<argument> + arg_sep + dsl::p<argument>;
    static constexpr auto value = lexy::new_<shell::cmd_set, shell::command>;
};

// Parses one of three commands.
struct command
{
    struct unknown_command
    {
        static constexpr auto name = "unknown command";
    };
    struct trailing_args
    {
        static constexpr auto name = "trailing command arguments";
    };

    static constexpr auto rule = [] {
        auto unknown  = dsl::error<unknown_command>(identifier);
        auto commands = dsl::p<cmd_exit> | dsl::p<cmd_echo> //
                        | dsl::p<cmd_set> | unknown;

        // Allow an optional argument separator after the final command,
        // but then there should not be any other arguments after that.
        return commands + dsl::if_(arg_sep) + dsl::peek(dsl::eol).error<trailing_args>;
    }();
    static constexpr auto value = lexy::forward<shell::command>;
};

//=== the directive ===//
// Parse an optional parsing directive with trailing separator.
// Note that it doesn't parse the following command, this is done manually.
struct directive
{
    struct unknown_directive
    {
        static constexpr auto name = "unknown directive";
    };

    // Map pre-defined directives.
    static constexpr auto directives
        = lexy::symbol_table<shell::directive>                        //
              .map<LEXY_SYMBOL("execute")>(shell::directive::execute) //
              .map<LEXY_SYMBOL("trace")>(shell::directive::trace)     //
              .map<LEXY_SYMBOL("vars")>(shell::directive::vars);

    static constexpr auto rule = [] {
        auto pattern   = dsl::identifier(dsl::ascii::alpha);
        auto directive = dsl::symbol<directives>(pattern).error<unknown_directive>;

        // A directive is optional, but it also consumes the argument separator if parsed.
        return dsl::opt(dsl::colon >> directive + dsl::if_(arg_sep));
    }();

    // Forward the existing directive but default to execute.
    static constexpr auto value
        = lexy::bind(lexy::forward<shell::directive>, lexy::_1 or shell::directive::execute);
};
} // namespace grammar

#ifndef LEXY_TEST
#    include <lexy/action/parse.hpp>
#    include <lexy/action/scan.hpp>
#    include <lexy/action/trace.hpp>
#    include <lexy_ext/report_error.hpp>

int main()
{
    for (shell::interpreter intp; intp.shell.is_open();)
    {
        // We repeatedly prompt for a new line.
        // Note: everytime we do it, the memory of the previous line is cleared.
        auto input = intp.shell.prompt_for_input();

        // Use a scanner to handle the directives.
        auto scanner = lexy::scan(input, lexy_ext::report_error);
        // Parse a directive.
        auto directive = scanner.parse<grammar::directive>();
        if (!scanner)
            continue;

        auto exit = false;
        switch (directive.value())
        {
        case shell::directive::execute: {
            // Parse the command in the remaining input...
            auto result = lexy::parse<grammar::command>(scanner.remaining_input(), intp,
                                                        lexy_ext::report_error);
            if (result)
                // ... and execute it.
                exit = result.value()->execute(intp);
            break;
        }

        case shell::directive::trace:
            // Trace the command in the remaining input.
            lexy::trace_to<grammar::command>(intp.shell.write_message().output_iterator(),
                                             scanner.remaining_input(), {lexy::visualize_fancy});
            break;

        case shell::directive::vars: {
            // Write all variables.
            auto writer = intp.shell.write_message();
            for (auto [name, value] : intp.variables)
            {
                writer(name.data(), name.length());
                writer(" = ");
                writer(value.data(), value.length());
                writer("\n");
            }
        }
        }

        if (exit)
            break;
    }
}
#endif

