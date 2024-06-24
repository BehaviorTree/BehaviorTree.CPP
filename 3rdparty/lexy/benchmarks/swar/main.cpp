// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#define ANKERL_NANOBENCH_IMPLEMENT
#include "swar.hpp"

#include <lexy/_detail/code_point.hpp>
#include <random>

namespace
{
std::default_random_engine engine;

char random_ascii()
{
    static std::uniform_int_distribution<std::uint_least32_t> dist(0x00, 0x7F);
    return static_cast<char>(dist(engine));
}

char32_t random_unicode()
{
    static std::uniform_int_distribution<std::uint_least32_t> dist(0x80, 0x10'FFFF);
    return static_cast<char32_t>(dist(engine));
}

float random01()
{
    static std::uniform_real_distribution<float> dist(0, 1);
    return dist(engine);
}
} // namespace

lexy::buffer<lexy::utf8_encoding> random_buffer(std::size_t size, float unicode_ratio)
{
    lexy::buffer<lexy::utf8_encoding>::builder builder(size);
    for (auto i = std::size_t(0); i != size;)
    {
        if (random01() < unicode_ratio)
        {
            auto cp = random_unicode();

            LEXY_CHAR8_T buffer[4];
            auto cp_size = lexy::_detail::encode_code_point<lexy::utf8_encoding>(cp, buffer, 4);
            if (i + cp_size < size)
            {
                std::memcpy(builder.data() + i, buffer, cp_size);
                i += cp_size;
            }
        }
        else
        {
            builder.data()[i] = static_cast<LEXY_CHAR8_T>(random_ascii());
            ++i;
        }
    }
    return LEXY_MOV(builder).finish();
}
lexy::buffer<lexy::utf8_encoding> repeat_buffer_padded(std::size_t size, const char* str)
{
    lexy::buffer<lexy::utf8_encoding>::builder builder(size);
    for (auto i = std::size_t(0); i != size;)
    {
        auto remaining = size - i;
        if (random01() >= 0.5)
        {
            auto length = std::size_t(random_ascii());
            if (length > remaining)
                length = remaining;
            std::memset(builder.data() + i, static_cast<unsigned char>(length), length);
            i += length;
        }
        else if (std::strlen(str) < remaining)
        {
            std::memcpy(builder.data() + i, str, std::strlen(str)); // NOLINT
            i += std::strlen(str);
        }
    }
    return LEXY_MOV(builder).finish();
}

std::size_t bm_any(ankerl::nanobench::Bench& b);
std::size_t bm_digits(ankerl::nanobench::Bench& b);
std::size_t bm_delimited(ankerl::nanobench::Bench& b);
std::size_t bm_identifier(ankerl::nanobench::Bench& b);
std::size_t bm_lit(ankerl::nanobench::Bench& b);
std::size_t bm_until(ankerl::nanobench::Bench& b);

int main(int argc, char* argv[])
{
    ankerl::nanobench::Bench b;
    if (argc == 1 || argv[1] == std::string_view("any"))
        bm_any(b);
    if (argc == 1 || argv[1] == std::string_view("delimited"))
        bm_delimited(b);
    if (argc == 1 || argv[1] == std::string_view("digits"))
        bm_digits(b);
    if (argc == 1 || argv[1] == std::string_view("identifier"))
        bm_identifier(b);
    if (argc == 1 || argv[1] == std::string_view("lit"))
        bm_lit(b);
    if (argc == 1 || argv[1] == std::string_view("until"))
        bm_until(b);
}

