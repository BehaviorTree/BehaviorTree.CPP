// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/file.hpp>
#include <nlohmann/json.hpp>

bool json_nlohmann(const lexy::buffer<lexy::utf8_encoding>& input)
{
    return nlohmann::json::accept(input.data(), input.data() + input.size());
}

