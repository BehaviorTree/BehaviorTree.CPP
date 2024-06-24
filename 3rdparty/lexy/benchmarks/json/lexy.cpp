// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/validate.hpp>
#include <lexy/input/file.hpp>

#define LEXY_TEST
#include "../../examples/json.cpp"

bool json_lexy(const lexy::buffer<lexy::utf8_encoding>& input)
{
    return lexy::validate<grammar::json>(input, lexy::noop).is_success();
}

