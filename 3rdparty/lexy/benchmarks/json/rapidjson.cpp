// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/file.hpp>
#include <rapidjson/reader.h>

bool json_rapid(const lexy::buffer<lexy::utf8_encoding>& input)
{
    rapidjson::MemoryStream stream(reinterpret_cast<const char*>(input.data()), input.size());
    rapidjson::BaseReaderHandler<> handler;

    rapidjson::Reader reader;
    return !reader.Parse(stream, handler).IsError();
}

