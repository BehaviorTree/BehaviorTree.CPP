// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/file.hpp>

bool json_baseline(const lexy::buffer<lexy::utf8_encoding>& input)
{
    // Just do something with the input.
    std::size_t sum = 0;
    for (auto ptr = input.data(); ptr != input.data() + input.size(); ++ptr)
        sum += *ptr;
    return sum % 11 == 0;
}

