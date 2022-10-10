/*  Copyright (C) 2022 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <charconv>

#include "lexy/action/parse.hpp"
#include "lexy/callback.hpp"
#include "lexy/dsl.hpp"
#include "lexy_ext/report_error.hpp"

#include "behaviortree_cpp/utils/safe_any.hpp"

namespace BT::Grammar
{
namespace dsl = lexy::dsl;

//----------
struct Integer : lexy::token_production
{
    struct integer
    {
        static constexpr auto rule = dsl::sign + dsl::integer<int64_t>;
        static constexpr auto value = lexy::as_integer<int64_t>;
    };

    struct invalid_suffix
    {
        static constexpr auto name = "invalid suffix on integer literal";
    };

    static constexpr auto rule = [] {
        auto hex_integer = (LEXY_LIT("0x") | LEXY_LIT("0X")) >> dsl::integer<int64_t, dsl::hex>;
        auto regular_integer = dsl::peek(dsl::lit_c<'-'> /
                                         dsl::lit_c<'+'> /
                                         dsl::digit<>) >> dsl::p<integer>;
        auto suffix_error = dsl::peek_not(dsl::period /
                                          dsl::ascii::alpha /
                                          dsl::ascii::alpha_underscore).error<invalid_suffix>;
        return ( hex_integer | regular_integer ) >> suffix_error;
    }();

    static constexpr auto value = lexy::construct<int64_t>;
};

//----------
struct Real : lexy::token_production
{
    struct invalid_suffix
    {
        static constexpr auto name = "invalid suffix on double literal";
    };

    static constexpr auto rule = [] {
        auto integer_part = dsl::sign + dsl::digits<>;
        auto fraction = dsl::period >> dsl::digits<>;
        auto exponent = (dsl::lit_c<'e'> / dsl::lit_c<'E'>) >> (dsl::sign + dsl::digits<>);

        auto suffix_error = dsl::peek_not(dsl::period /
                                          dsl::ascii::alpha /
                                          dsl::ascii::alpha_underscore).error<invalid_suffix>;

        auto real_part = (fraction >> dsl::if_(exponent) | exponent) >> suffix_error;
        auto real_number = dsl::token(integer_part + real_part);
        return dsl::capture(real_number);
    }();

    static constexpr auto value
        = lexy::as_string<std::string>
          | lexy::callback<BT::Any>([](std::string&& str) {
              return BT::Any(std::stod(str));
            });
};

//----------
//struct Variable : lexy::token_production
//{
//  static constexpr auto rule = dsl::identifier(dsl::unicode::xid_start_underscore, dsl::unicode::xid_continue);
//  static constexpr auto value = lexy::as_string<std::string>;
//};

//----------
struct StringLiteral : lexy::token_production
{
  static constexpr auto rule = dsl::single_quoted(dsl::ascii::character) | dsl::quoted(dsl::ascii::character);
  static constexpr auto value = lexy::as_string<std::string>;
};

//----------
struct BooleanLiteral : lexy::token_production
{
  struct True
  {
    static constexpr auto rule = LEXY_LIT("true");
    static constexpr auto value = lexy::constant(1);
  };
  struct False
  {
    static constexpr auto rule = LEXY_LIT("false");
    static constexpr auto value = lexy::constant(0);
  };

  static constexpr auto rule = dsl::p<True> | dsl::p<False>;
  static constexpr auto value = lexy::construct<BT::Any>;
};

//----------
struct AnyValue  : lexy::token_production
{
  static constexpr auto rule = dsl::p<BooleanLiteral> | dsl::p<StringLiteral> | dsl::p<Real> | dsl::p<Integer> ;
  static constexpr auto value = lexy::construct<BT::Any>;
};

} // end namespace Grammar

