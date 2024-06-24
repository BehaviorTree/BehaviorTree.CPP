// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <lexy/input/file.hpp>

using namespace boost::json;

// Adapted from https://github.com/boostorg/json/blob/develop/example/validate.cpp
class null_parser
{
    struct handler
    {
        constexpr static std::size_t max_object_size = std::size_t(-1);
        constexpr static std::size_t max_array_size  = std::size_t(-1);
        constexpr static std::size_t max_key_size    = std::size_t(-1);
        constexpr static std::size_t max_string_size = std::size_t(-1);

        bool on_document_begin(error_code&)
        {
            return true;
        }
        bool on_document_end(error_code&)
        {
            return true;
        }
        bool on_object_begin(error_code&)
        {
            return true;
        }
        bool on_object_end(std::size_t, error_code&)
        {
            return true;
        }
        bool on_array_begin(error_code&)
        {
            return true;
        }
        bool on_array_end(std::size_t, error_code&)
        {
            return true;
        }
        bool on_key_part(string_view, std::size_t, error_code&)
        {
            return true;
        }
        bool on_key(string_view, std::size_t, error_code&)
        {
            return true;
        }
        bool on_string_part(string_view, std::size_t, error_code&)
        {
            return true;
        }
        bool on_string(string_view, std::size_t, error_code&)
        {
            return true;
        }
        bool on_number_part(string_view, error_code&)
        {
            return true;
        }
        bool on_int64(std::int64_t, string_view, error_code&)
        {
            return true;
        }
        bool on_uint64(std::uint64_t, string_view, error_code&)
        {
            return true;
        }
        bool on_double(double, string_view, error_code&)
        {
            return true;
        }
        bool on_bool(bool, error_code&)
        {
            return true;
        }
        bool on_null(error_code&)
        {
            return true;
        }
        bool on_comment_part(string_view, error_code&)
        {
            return true;
        }
        bool on_comment(string_view, error_code&)
        {
            return true;
        }
    };

    basic_parser<handler> p_;

public:
    null_parser() : p_(parse_options()) {}

    bool validate(const lexy::buffer<lexy::utf8_encoding>& input)
    {
        error_code ec;
        auto       n
            = p_.write_some(false, reinterpret_cast<const char*>(input.data()), input.size(), ec);
        if (!ec && n < input.size())
            ec = error::extra_data;
        return !!ec;
    }
};

bool json_boost(const lexy::buffer<lexy::utf8_encoding>& input)
{
    return null_parser().validate(input);
}

