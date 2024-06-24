// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef TESTS_LEXY_DSL_VERIFY_HPP_INCLUDED
#define TESTS_LEXY_DSL_VERIFY_HPP_INCLUDED

#include <doctest/doctest.h>
#include <lexy/action/base.hpp>
#include <lexy/action/match.hpp>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/dsl/any.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/token.hpp>
#include <lexy/visualize.hpp>

#if (defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 7)                                    \
    || (!defined(__clang__) && defined(_MSC_VER))
// GCC 7 and MSVC are too buggy.
#    define LEXY_DISABLE_CONSTEXPR_TESTS
#endif

//=== conveniences ===//
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/literal.hpp>

namespace lexy_test
{}

using namespace lexy_test;

namespace dsl = lexy::dsl;

//=== doctest toString ===//
namespace doctest
{
struct _append_iterator
{
    doctest::String* _result;

    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    _append_iterator& operator=(char c)
    {
        *_result += doctest::String(&c, 1);
        return *this;
    }
};

template <>
struct StringMaker<lexy::code_point>
{
    static String convert(lexy::code_point cp)
    {
        doctest::String result;
        lexy::visualize_to(_append_iterator{&result}, cp,
                           lexy::visualization_options{lexy::visualize_space});
        return result;
    }
};

template <typename Reader>
struct StringMaker<lexy::lexeme<Reader>>
{
    static String convert(lexy::lexeme<Reader> lex)
    {
        doctest::String result;
        lexy::visualize_to(_append_iterator{&result}, lex,
                           lexy::visualization_options{lexy::visualize_space});
        return result;
    }
};
} // namespace doctest

//=== rule equivalence ===//
namespace lexy_test
{
template <typename RuleA, typename RuleB>
constexpr bool equivalent_rules(RuleA, RuleB)
{
    return std::is_same_v<
               RuleA, RuleB> || std::is_base_of_v<RuleA, RuleB> || std::is_base_of_v<RuleB, RuleA>;
}
} // namespace lexy_test

//=== verify ===//
namespace lexy::parse_events
{
struct debug;
}

namespace lexy_test
{
struct test_production
{
    static constexpr auto name = "test_production";
};

template <typename Rule>
struct production_for
{
    static constexpr auto rule = Rule{};
};
template <typename Rule>
struct test_production_for : production_for<Rule>, test_production
{};

template <typename Production>
constexpr auto is_test_production = std::is_base_of_v<test_production, Production>;

class test_trace
{
public:
    explicit test_trace(std::nullptr_t) : _level(0)
    {
        _trace += "\n";
    }

    test_trace() : test_trace(nullptr)
    {
        production("test_production");
    }

    test_trace& production(const char* name)
    {
        prefix();
        _trace += name;
        _trace += "\n";

        ++_level;

        return *this;
    }
    test_trace& recovery()
    {
        return production("error recovery");
    }

    test_trace& token(const char* kind, const char* spelling)
    {
        prefix();

        _trace += kind;
        _trace += ": ";
        _trace += spelling;
        _trace += "\n";

        return *this;
    }
    test_trace& token(const char* spelling)
    {
        return token("token", spelling);
    }
    test_trace& literal(const char* spelling)
    {
        return token("literal", spelling);
    }
    test_trace& digits(const char* spelling)
    {
        return token("digits", spelling);
    }
    test_trace& whitespace(const char* spelling)
    {
        return token("whitespace", spelling);
    }
    test_trace& error_token(const char* spelling)
    {
        return token("error token", spelling);
    }
    test_trace& eof()
    {
        return token("EOF", "");
    }
    test_trace& position()
    {
        return token("position", "");
    }

    test_trace& operation_chain()
    {
        return production("operation chain");
    }
    test_trace& operation(const char* name)
    {
        return token("operation", name);
    }

    test_trace& backtracked(const char* spelling)
    {
        return token("backtracked", spelling);
    }

    test_trace& debug(const char* message)
    {
        return token("debug", message);
    }

    test_trace& error(std::size_t begin, std::size_t end, const char* message)
    {
        prefix();

        _trace += "error: ";
        _trace += message;

        _trace += " @";
        _trace += doctest::toString(begin);
        _trace += "-";
        _trace += doctest::toString(end);

        _trace += "\n";

        return *this;
    }
    test_trace& expected_literal(std::size_t pos, const doctest::String& literal, std::size_t index)
    {
        prefix();

        _trace += "error: expected '";
        _trace += literal;
        _trace += "'";

        _trace += " @";
        _trace += doctest::toString(pos);
        _trace += "-";
        _trace += doctest::toString(pos + index);

        _trace += "\n";

        return *this;
    }
    template <std::size_t N>
    test_trace& expected_literal(std::size_t pos, const char (&literal)[N], std::size_t index)
    {
        return expected_literal(pos, doctest::String(literal, N - 1), index);
    }
    test_trace& expected_keyword(std::size_t begin, std::size_t end, const char* keyword)
    {
        prefix();

        _trace += "error: expected keyword '";
        _trace += keyword;
        _trace += "'";

        _trace += " @";
        _trace += doctest::toString(begin);
        _trace += "-";
        _trace += doctest::toString(end);

        _trace += "\n";

        return *this;
    }
    test_trace& expected_char_class(std::size_t pos, const char* c)
    {
        prefix();

        _trace += "error: expected ";
        _trace += c;

        _trace += " @";
        _trace += doctest::toString(pos);

        _trace += "\n";

        return *this;
    }

    test_trace& finish()
    {
        --_level;
        return *this;
    }
    test_trace& cancel()
    {
        prefix();
        _trace += "cancel\n";
        --_level;

        return *this;
    }

    friend doctest::String toString(const test_trace& self)
    {
        return self._trace + doctest::String("\n         ");
    }

    friend bool operator==(const test_trace& lhs, const test_trace& rhs)
    {
        return lhs._trace == rhs._trace;
    }

private:
    void prefix()
    {
        // First indent to align output regardless of level.
        _trace += "            ";

        // Then indent child nodes.
        if (_level > 0)
        {
            for (auto i = 0; i != _level - 1; ++i)
                _trace += "  ";
            _trace += "- ";
        }
    }

    doctest::String _trace;
    int             _level;
};

struct test_result
{
    enum
    {
        success,
        recovered_error,
        fatal_error,
    } status;

    int        value;
    test_trace trace;
};

template <typename Input, typename Callback>
struct test_handler
{
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    test_handler(const Input& input, Callback cb)
    : _trace(nullptr), _cb(cb), _had_error(false), _begin(input.reader().position()),
      _last_token(_begin)
    {}

    template <typename Production>
    class event_handler
    {
    public:
        void on(test_handler& handler, lexy::parse_events::production_start, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.production(lexy::production_name<Production>());
        }
        void on(test_handler& handler, lexy::parse_events::production_finish, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.finish();
        }
        void on(test_handler& handler, lexy::parse_events::production_cancel, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.cancel();
        }

        auto on(test_handler& handler, lexy::parse_events::operation_chain_start, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.operation_chain();
            return 0;
        }
        template <typename Operation>
        void on(test_handler& handler, lexy::parse_events::operation_chain_op, Operation,
                iterator      pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.operation(lexy::production_name<Operation>());
        }
        void on(test_handler& handler, lexy::parse_events::operation_chain_finish, int&&,
                iterator      pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.finish();
        }

        template <typename TK>
        void on(test_handler& handler, lexy::parse_events::token, TK _kind, iterator begin,
                iterator end)
        {
            auto kind = lexy::token_kind(_kind);
            if (kind.ignore_if_empty() && begin == end)
                return;

            auto lex      = lexy::lexeme_for<Input>(begin, end);
            auto spelling = doctest::toString(lex);

            handler._trace.token(kind.name(), spelling.c_str());

            CHECK(handler._last_token == begin);
            handler._last_token = end;
        }
        void on(test_handler& handler, lexy::parse_events::backtracked, iterator begin,
                iterator end)
        {
            CHECK(handler._last_token == begin);

            if (begin != end)
            {
                auto lex      = lexy::lexeme_for<Input>(begin, end);
                auto spelling = doctest::toString(lex);

                handler._trace.backtracked(spelling.c_str());
            }
        }

        template <typename Reader, typename Tag>
        void on(test_handler&                   handler, lexy::parse_events::error,
                const lexy::error<Reader, Tag>& error)
        {
            auto begin = lexy::_detail::range_size(handler._begin, error.begin());
            auto end   = lexy::_detail::range_size(handler._begin, error.end());
            handler._trace.error(begin, end, error.message());

            handler._had_error = true;
        }
        template <typename Reader>
        void on(test_handler& handler, lexy::parse_events::error,
                const lexy::error<Reader, lexy::expected_literal>& error)
        {
            auto pos = lexy::_detail::range_size(handler._begin, error.position());
            auto string
                = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                error.length());
            handler._trace.expected_literal(pos, doctest::toString(string), error.index());

            handler._had_error = true;
        }
        template <typename Reader>
        void on(test_handler& handler, lexy::parse_events::error,
                const lexy::error<Reader, lexy::expected_keyword>& error)
        {
            auto begin = lexy::_detail::range_size(handler._begin, error.begin());
            auto end   = lexy::_detail::range_size(handler._begin, error.end());
            auto string
                = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                error.length());
            handler._trace.expected_keyword(begin, end, doctest::toString(string).c_str());

            handler._had_error = true;
        }
        template <typename Reader>
        void on(test_handler& handler, lexy::parse_events::error,
                const lexy::error<Reader, lexy::expected_char_class>& error)
        {
            auto pos = lexy::_detail::range_size(handler._begin, error.position());
            handler._trace.expected_char_class(pos, error.name());

            handler._had_error = true;
        }

        void on(test_handler& handler, lexy::parse_events::recovery_start, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.recovery();
        }
        void on(test_handler& handler, lexy::parse_events::recovery_finish, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.finish();
        }
        void on(test_handler& handler, lexy::parse_events::recovery_cancel, iterator pos)
        {
            CHECK(handler._last_token == pos);
            handler._trace.cancel();
        }

        void on(test_handler& handler, const lexy::parse_events::debug&, iterator pos,
                const char* str)
        {
            CHECK(handler._last_token == pos);
            handler._trace.token("debug", str);
        }
    };

    template <typename Production, typename State>
    class value_callback
    {
        static_assert(std::is_same_v<State, test_handler>);

    public:
        constexpr explicit value_callback(const State* handler) : _handler(handler) {}

        using return_type = std::conditional_t<is_test_production<Production>, int, Production>;

        constexpr auto sink() const
        {
            if constexpr (is_test_production<Production> && lexy::is_sink<decltype(_handler->_cb)>)
                return _handler->_cb.sink();
            else
                return lexy::count.sink();
        }

        template <typename... Args>
        constexpr return_type operator()([[maybe_unused]] Args&&... args) const
        {
            if constexpr (is_test_production<Production>)
                return _handler->_cb(_handler->_begin, LEXY_FWD(args)...);
            else
                return Production{};
        }

    private:
        const test_handler* _handler;
    };

    template <typename T>
    constexpr auto get_result(bool rule_parse_result, int result) &&
    {
        if (rule_parse_result)
            return test_result{_had_error ? test_result::recovered_error : test_result::success,
                               result, LEXY_MOV(_trace)};
        else
            return test_result{test_result::fatal_error, -1, LEXY_MOV(_trace)};
    }
    template <typename T>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        return LEXY_MOV(*this).template get_result<T>(rule_parse_result, -1);
    }

    constexpr iterator begin() const
    {
        return _begin;
    }

private:
    test_trace _trace;

    Callback _cb;
    bool     _had_error;

    iterator _begin, _last_token;
};

constexpr auto token_callback = [](auto) { return 0; };

template <typename Production, typename Input, typename Callback>
test_result verify(const Input& input, Callback cb)
{
    INFO(lexy::lexeme_for<Input>(input.reader().position(), [&] {
        auto                                                         reader = input.reader();
        lexy::token_parser_for<decltype(dsl::any), decltype(reader)> parser(reader);
        parser.try_parse(reader);
        return parser.end;
    }()));

    test_handler<Input, Callback> handler(input, cb);
    auto                          reader = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), &handler, reader);
}

template <typename Rule, typename Input, typename Callback>
test_result verify(Rule, const Input& input, Callback cb)
{
    return verify<test_production_for<Rule>>(input, cb);
}

template <typename Input, typename = lexy::input_reader<Input>>
constexpr auto _get_input(const Input& input)
{
    return input;
}
template <typename CharT>
constexpr auto _get_input(const CharT* str)
{
    return lexy::zstring_input(str);
}
template <typename CharT>
constexpr auto _get_input(const CharT* str, std::size_t length)
{
    return lexy::string_input(str, length);
}
template <typename Encoding, typename CharT>
constexpr auto _get_input(Encoding, const CharT* str)
{
    return lexy::zstring_input<Encoding>(str);
}
template <typename Encoding, typename = typename Encoding::int_type>
constexpr auto _get_input(Encoding)
{
    return lexy::string_input<Encoding>();
}
template <typename Encoding, typename... CharT,
          typename
          = std::enable_if_t<(std::is_convertible_v<CharT, typename Encoding::char_type> && ...)
                             || (std::is_same_v<CharT, lexy::code_point> && ...)>>
constexpr auto _get_input(Encoding, CharT... cs)
{
    using char_type = typename Encoding::char_type;

    struct input
    {
        char_type   array[sizeof...(CharT) * 4];
        std::size_t size;

        constexpr input(CharT... cs) : array{}, size{}
        {
            auto ptr = array;
            if constexpr ((std::is_same_v<CharT, lexy::code_point> && ...))
            {
                ((ptr += lexy::_detail::encode_code_point<Encoding>(cs.value(), ptr, 4)), ...);
            }
            else
            {
                ((*ptr++ = static_cast<char_type>(cs)), ...);
            }
            size = static_cast<std::size_t>(ptr - array);
        }

        input(const input&) = delete;
        input& operator=(const input&) = delete;

        constexpr auto reader() const&
        {
            return lexy::_range_reader<Encoding>(array, array + sizeof...(CharT));
        }
    };

    return input(cs...);
}
} // namespace lexy_test

#define LEXY_VERIFY_RUNTIME_P(Prod, ...)                                                           \
    lexy_test::verify<Prod>(lexy_test::_get_input(__VA_ARGS__), callback)
#define LEXY_VERIFY_RUNTIME(...)                                                                   \
    LEXY_VERIFY_RUNTIME_P(lexy_test::test_production_for<decltype(rule)>, __VA_ARGS__)

#ifndef LEXY_DISABLE_CONSTEXPR_TESTS

#    define LEXY_VERIFY_P(Prod, ...)                                                               \
        [&] {                                                                                      \
            constexpr auto _input   = lexy_test::_get_input(__VA_ARGS__);                          \
            constexpr auto _matches = lexy::match<Prod>(_input);                                   \
                                                                                                   \
            auto _result = lexy_test::verify<Prod>(_input, callback);                              \
            if (_matches)                                                                          \
                CHECK(_result.status == lexy_test::test_result::success);                          \
            else if (_matches)                                                                     \
                CHECK(_result.status != lexy_test::test_result::success);                          \
            return _result;                                                                        \
        }()
#else

#    define LEXY_VERIFY_P(Prod, ...) LEXY_VERIFY_RUNTIME_P(Prod, __VA_ARGS__)

#endif

#define LEXY_VERIFY(...) LEXY_VERIFY_P(lexy_test::test_production_for<decltype(rule)>, __VA_ARGS__)

#endif // TESTS_LEXY_DSL_VERIFY_HPP_INCLUDED

