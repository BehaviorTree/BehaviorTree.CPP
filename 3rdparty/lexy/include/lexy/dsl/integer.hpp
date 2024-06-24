// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_INTEGER_HPP_INCLUDED
#define LEXY_DSL_INTEGER_HPP_INCLUDED

#include <climits>

#include <lexy/_detail/assert.hpp>
#include <lexy/code_point.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/digit.hpp>

namespace lexy
{
// Number of digits to express the given value.
template <typename Integer>
constexpr std::size_t _digit_count(int radix, Integer value)
{
    LEXY_PRECONDITION(value >= Integer(0));

    if (value == 0)
        return 1;

    std::size_t result = 0;
    while (value > 0)
    {
        value = Integer(value / Integer(radix));
        ++result;
    }
    return result;
}

template <typename T>
struct integer_traits
{
    using type = T;

    static constexpr auto is_bounded = true;

    static constexpr auto _max = [] {
        if constexpr (std::is_same_v<T, char>)
            return CHAR_MAX; // NOLINT
        else if constexpr (std::is_same_v<T, signed char>)
            return SCHAR_MAX;
        else if constexpr (std::is_same_v<T, unsigned char>)
            return UCHAR_MAX; // NOLINT
        else if constexpr (std::is_same_v<T, wchar_t>)
            return WCHAR_MAX; // NOLINT
#if LEXY_HAS_CHAR8_T
        else if constexpr (std::is_same_v<T, char8_t>)
            return UCHAR_MAX; // NOLINT
#endif
        else if constexpr (std::is_same_v<T, char16_t>)
            return UINT_LEAST16_MAX;
        else if constexpr (std::is_same_v<T, char32_t>)
            return UINT_LEAST32_MAX;
        else if constexpr (std::is_same_v<T, signed short>)
            return SHRT_MAX;
        else if constexpr (std::is_same_v<T, unsigned short>)
            return USHRT_MAX;
        else if constexpr (std::is_same_v<T, signed int>)
            return INT_MAX;
        else if constexpr (std::is_same_v<T, unsigned int>)
            return UINT_MAX;
        else if constexpr (std::is_same_v<T, signed long>)
            return LONG_MAX;
        else if constexpr (std::is_same_v<T, unsigned long>)
            return ULONG_MAX;
        else if constexpr (std::is_same_v<T, signed long long>)
            return LLONG_MAX;
        else if constexpr (std::is_same_v<T, unsigned long long>)
            return ULLONG_MAX;
#ifdef __SIZEOF_INT128__
        else if constexpr (std::is_same_v<T, __int128_t>)
            return __int128_t(~__uint128_t{} >> 1);
        else if constexpr (std::is_same_v<T, __uint128_t>)
            return ~__uint128_t{};
#endif
        else
            static_assert(_detail::error<T>,
                          "specialize integer_traits for your custom integer types");
    }();
    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, _max);

    template <int Radix>
    static constexpr void add_digit_unchecked(T& result, unsigned digit)
    {
        result = T(result * T(Radix) + T(digit));
    }

    template <int Radix>
    static constexpr bool add_digit_checked(T& result, unsigned digit)
    {
        constexpr auto can_use_unsigned = [] {
            if constexpr (sizeof(T) >= sizeof(unsigned))
                // If it's bigger or of the same size as unsigned, we can't use unsigned.
                return false;
            else
            {
                // We can do it if the worst-case does not overflow unsigned.
                auto worst_case = static_cast<unsigned>(_max);
                return integer_traits<unsigned>::add_digit_checked<Radix>(worst_case, Radix - 1);
            }
        }();

        // Check whether we can do an optimization for small integers,
        // where we do the operation on unsigned and check later.
        if constexpr (can_use_unsigned)
        {
            // This can't overflow, we've checked it above.
            auto value = static_cast<unsigned>(result) * Radix + digit;

            // Check whether the final value can fit.
            if (value > static_cast<unsigned>(_max))
                return false;
            else
            {
                result = static_cast<T>(value);
                return true;
            }
        }
        else
        {
            // result *= Radix
            constexpr auto max_per_radix = T(_max / Radix);
            if (result > max_per_radix)
                return false;
            result = T(result * Radix);

            // result += digit
            if (result > T(_max - digit))
                return false;
            result = T(result + T(digit));

            return true;
        }
    }
};

template <>
struct integer_traits<code_point>
{
    using type = code_point;

    static constexpr auto is_bounded = true;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, 0x10'FFFF);

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        std::uint_least32_t value = result.value();
        integer_traits<std::uint_least32_t>::add_digit_unchecked<Radix>(value, digit);
        result = code_point(value);
    }
    template <int Radix>
    static constexpr bool add_digit_checked(type& result, unsigned digit)
    {
        std::uint_least32_t value = result.value();
        if (!integer_traits<std::uint_least32_t>::add_digit_checked<Radix>(value, digit))
            return false;
        result = code_point(value);
        return result.is_valid();
    }
};

template <typename T>
struct unbounded
{};
template <typename T>
struct integer_traits<unbounded<T>>
{
    using type                       = typename integer_traits<T>::type;
    static constexpr auto is_bounded = false;

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        integer_traits<T>::template add_digit_unchecked<Radix>(result, digit);
    }
};

template <typename T, T Max>
struct bounded
{};
template <typename T, T Max>
struct integer_traits<bounded<T, Max>>
{
    using type                       = typename integer_traits<T>::type;
    static constexpr auto is_bounded = true;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, Max);

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        integer_traits<T>::template add_digit_unchecked<Radix>(result, digit);
    }
    template <int Radix>
    static constexpr bool add_digit_checked(type& result, unsigned digit)
    {
        return integer_traits<T>::template add_digit_checked<Radix>(result, digit) && result <= Max;
    }
};
} // namespace lexy

namespace lexy
{
struct integer_overflow
{
    static LEXY_CONSTEVAL auto name()
    {
        return "integer overflow";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename T>
constexpr bool _is_bounded = lexy::integer_traits<T>::is_bounded;

template <typename T, std::size_t N, int Radix>
constexpr bool _ndigits_can_overflow()
{
    using traits         = lexy::integer_traits<T>;
    auto max_digit_count = traits::template max_digit_count<Radix>;
    // We don't know whether the maximal value is a power of Radix,
    // so we have to be conservative and don't rule out overflow on the same count.
    return N >= max_digit_count;
}

// Parses T in the Base without checking for overflow.
template <typename T, typename Base>
struct _unbounded_integer_parser
{
    using traits = lexy::integer_traits<T>;
    using base   = Base;

    static constexpr auto radix = Base::digit_radix;

    struct result_type
    {
        typename traits::type value;
        std::false_type       overflow;
    };

    template <typename Iterator>
    static constexpr result_type parse(Iterator cur, Iterator end)
    {
        typename traits::type value(0);

        // Just parse digits until we've run out of digits.
        while (cur != end)
        {
            auto digit = Base::digit_value(*cur++);
            if (digit >= Base::digit_radix)
                // Skip digit separator.
                continue;

            traits::template add_digit_unchecked<radix>(value, digit);
        }

        return {value, {}};
    }
};

// Parses T in the Base while checking for overflow.
template <typename T, typename Base, bool AssumeOnlyDigits>
struct _bounded_integer_parser
{
    using traits = lexy::integer_traits<T>;
    using base   = Base;

    static constexpr auto radix           = Base::digit_radix;
    static constexpr auto max_digit_count = traits::template max_digit_count<radix>;
    static_assert(max_digit_count > 1, "integer must be able to store all possible digit values");

    struct result_type
    {
        typename traits::type value;
        bool                  overflow;
    };

    template <typename Iterator>
    static constexpr result_type parse(Iterator cur, Iterator end)
    {
        // Find the first non-zero digit.
        // Note that we always need a loop, even if leading zeros are not allowed:
        // error recovery might get them anyway.
        auto first_digit = 0u;
        while (true)
        {
            if (cur == end)
                return {typename traits::type(0), false};

            first_digit = Base::digit_value(*cur++);
            if (first_digit != 0 && first_digit < radix)
                break;
        }

        // At this point, we've parsed exactly one non-zero digit, so we can assign.
        auto value = typename traits::type(first_digit);

        // Handle at most the number of remaining digits.
        // Due to the fixed loop count, it is most likely unrolled.
        for (std::size_t digit_count = 1; digit_count < max_digit_count; ++digit_count)
        {
            // Find the next digit.
            auto digit = 0u;
            do
            {
                if (cur == end)
                    return {value, false};

                digit = Base::digit_value(*cur++);
                // If we can assume it's a digit, we don't need the comparison.
            } while (AssumeOnlyDigits ? false : digit >= Base::digit_radix);

            // We need to handle the last loop iteration special.
            // (The compiler will not generate a branch here.)
            if (digit_count == max_digit_count - 1)
            {
                // The last digit might overflow, so check for it.
                if (!traits::template add_digit_checked<radix>(value, digit))
                    return {value, true};
            }
            else
            {
                // Add the digit without checking as it can't overflow.
                traits::template add_digit_unchecked<radix>(value, digit);
            }
        }

        // If we've reached this point, we've parsed the maximal number of digits allowed.
        // Now we can only overflow if there are still digits left.
        return {value, cur != end};
    }
};
template <typename T, typename Base, bool AssumeOnlyDigits>
using _integer_parser
    = std::conditional_t<_is_bounded<T>, _bounded_integer_parser<T, Base, AssumeOnlyDigits>,
                         _unbounded_integer_parser<T, Base>>;

template <typename T, typename Digits>
struct _integer_parser_digits;
template <typename T, typename Base>
struct _integer_parser_digits<T, _digits<Base>>
{
    using type = _integer_parser<T, Base, true>;
};
template <typename T, typename Base>
struct _integer_parser_digits<T, _digits_t<Base>>
{
    using type = _integer_parser<T, Base, true>;
};
template <typename T, typename Base, typename Sep>
struct _integer_parser_digits<T, _digits_s<Base, Sep>>
{
    using type = _integer_parser<T, Base, false>;
};
template <typename T, typename Base, typename Sep>
struct _integer_parser_digits<T, _digits_st<Base, Sep>>
{
    using type = _integer_parser<T, Base, false>;
};
template <typename T, std::size_t N, typename Base>
struct _integer_parser_digits<T, _ndigits<N, Base>>
{
    using value_type = std::conditional_t<_ndigits_can_overflow<T, N, Base::digit_radix>(), T,
                                          lexy::unbounded<T>>;
    using type       = _integer_parser<value_type, Base, true>;
};
template <typename T, std::size_t N, typename Base, typename Sep>
struct _integer_parser_digits<T, _ndigits_s<N, Base, Sep>>
{
    using value_type = std::conditional_t<_ndigits_can_overflow<T, N, Base::digit_radix>(), T,
                                          lexy::unbounded<T>>;
    using type       = _integer_parser<value_type, Base, false>;
};

template <typename T, typename Digits>
using _integer_parser_for = typename _integer_parser_digits<T, Digits>::type;

template <typename Token, typename IntParser, typename Tag>
struct _int : _copy_base<Token>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           typename Reader::iterator begin,
                                           typename Reader::iterator end, Args&&... args)
        {
            auto [value, overflow] = IntParser::parse(begin, end);
            if (overflow)
            {
                // Raise error but recover.
                using tag = lexy::_detail::type_or<Tag, lexy::integer_overflow>;
                context.on(_ev::error{}, lexy::error<Reader, tag>(begin, end));
            }

            // Need to skip whitespace now as well.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)..., value);
        }
    };

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, Token{}, begin, end.position());
            reader.reset(end);

            return _pc<NextParser>::parse(context, reader, begin, end.position(),
                                          LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            if (lexy::token_parser_for<Token, Reader> parser(reader); parser.try_parse(reader))
            {
                context.on(_ev::token{}, typename Token::token_type{}, begin,
                           parser.end.position());
                reader.reset(parser.end);
            }
            else
            {
                parser.report_error(context, reader);
                reader.reset(parser.end);

                // To recover we try and skip additional digits.
                while (lexy::try_match_token(digit<typename IntParser::base>, reader))
                {
                }

                auto recovery_end = reader.position();
                if (begin == recovery_end)
                {
                    // We didn't get any digits; couldn't recover.
                    // We don't report error recovery events as nothing was done;
                    // we don't need to create an error token as nothing was consumed.
                    return false;
                }
                else
                {
                    // We've succesfully recovered, mark everything as digits.
                    context.on(_ev::recovery_start{}, begin);
                    context.on(_ev::token{}, lexy::digits_token_kind, begin, recovery_end);
                    context.on(_ev::recovery_finish{}, recovery_end);
                }
            }
            auto end = reader.position();

            return _pc<NextParser>::parse(context, reader, begin, end, LEXY_FWD(args)...);
        }
    };
};

template <typename T, typename Base>
struct _int_dsl : _int<_digits<lexy::_detail::type_or<Base, decimal>>,
                       _integer_parser_for<T, _digits<lexy::_detail::type_or<Base, decimal>>>, void>
{
    template <typename Digits>
    constexpr auto operator()(Digits) const
    {
        static_assert(lexy::is_token_rule<Digits>);
        if constexpr (std::is_void_v<Base>)
        {
            // Digits is a known rule as the user didn't specify Base.
            return _int<Digits, _integer_parser_for<T, Digits>, void>{};
        }
        else
        {
            // User has specified a base, so the digits are arbitrary.
            using parser = _integer_parser<T, Base, false>;
            return _int<Digits, parser, void>{};
        }
    }
};

/// Parses the digits matched by the rule into an integer type.
template <typename T, typename Base = void>
constexpr auto integer = _int_dsl<T, Base>{};
} // namespace lexyd

//=== code_point_id ===//
namespace lexy
{
struct invalid_code_point
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid code point";
    }
};
} // namespace lexy

namespace lexyd
{
/// Matches the integer value of a code point.
template <std::size_t N, typename Base = hex>
constexpr auto code_point_id = [] {
    using type = std::conditional_t<_ndigits_can_overflow<lexy::code_point, N, Base::digit_radix>(),
                                    lexy::code_point, lexy::unbounded<lexy::code_point>>;
    using parser = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, parser, lexy::invalid_code_point>{};
}();
} // namespace lexyd

//=== code_unit_id ===//
namespace lexy
{
struct invalid_code_unit
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid code unit";
    }
};
} // namespace lexy

namespace lexyd
{
/// Matches the integer value of a code unit.
template <typename Encoding, std::size_t N, typename Base = hex>
constexpr auto code_unit_id = [] {
    using char_type = typename Encoding::char_type;
    using type      = std::conditional_t<_ndigits_can_overflow<char_type, N, Base::digit_radix>(),
                                    char_type, lexy::unbounded<char_type>>;
    using parser    = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, parser, lexy::invalid_code_unit>{};
}();
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED

