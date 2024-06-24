// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/file.hpp>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/json.hpp>

namespace pegtl = tao::pegtl;

bool json_pegtl(const lexy::buffer<lexy::utf8_encoding>& _input)
{
    using grammar = pegtl::seq<pegtl::json::text, pegtl::eof>;

    pegtl::memory_input input(reinterpret_cast<const char*>(_input.data()), _input.size(), "");
    try
    {
        return pegtl::parse<grammar>(input);
    }
    catch (const pegtl::parse_error&)
    {
        return false;
    }
}

