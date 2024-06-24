// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/buffer_builder.hpp>

#include <doctest/doctest.h>

TEST_CASE("_detail::buffer_builder")
{
    lexy::_detail::buffer_builder<char> buffer;
    REQUIRE(buffer.read_size() == 0);
    REQUIRE(buffer.write_size() == buffer.capacity());
    REQUIRE(buffer.read_data() == buffer.write_data());

    std::strcpy(buffer.write_data(), "abc"); // NOLINT
    buffer.commit(3);
    REQUIRE(buffer.read_size() == 3);
    REQUIRE(buffer.write_size() == buffer.capacity() - 3);
    REQUIRE(buffer.read_data() + 3 == buffer.write_data());
    REQUIRE(std::strncmp(buffer.read_data(), "abc", 3) == 0);

    SUBCASE("grow partial")
    {
        auto old_cap = buffer.capacity();
        buffer.grow();
        REQUIRE(buffer.capacity() > old_cap);
        REQUIRE(buffer.read_size() == 3);
        REQUIRE(buffer.write_size() == buffer.capacity() - 3);
        REQUIRE(buffer.read_data() + 3 == buffer.write_data());
        REQUIRE(std::strncmp(buffer.read_data(), "abc", 3) == 0);
    }
    SUBCASE("grow full")
    {
        std::memset(buffer.write_data(), '!', buffer.write_size());
        buffer.commit(buffer.write_size());
        REQUIRE(buffer.read_size() == buffer.capacity());
        REQUIRE(buffer.write_size() == 0);
        REQUIRE(buffer.read_data() + buffer.capacity() == buffer.write_data());
        REQUIRE(std::strncmp(buffer.read_data(), "abc", 3) == 0);
        REQUIRE(buffer.read_data()[42] == '!');

        auto old_cap = buffer.capacity();
        buffer.grow();
        REQUIRE(buffer.capacity() > old_cap);
        REQUIRE(buffer.read_size() == old_cap);
        REQUIRE(buffer.write_size() == buffer.capacity() - old_cap);
        REQUIRE(buffer.read_data() + old_cap == buffer.write_data());
        REQUIRE(std::strncmp(buffer.read_data(), "abc", 3) == 0);
        REQUIRE(buffer.read_data()[42] == '!');
    }
    SUBCASE("grow twice")
    {
        auto old_cap = buffer.capacity();
        buffer.grow();
        REQUIRE(buffer.capacity() > old_cap);
        REQUIRE(buffer.read_size() == 3);
        REQUIRE(buffer.write_size() == buffer.capacity() - 3);
        REQUIRE(buffer.read_data() + 3 == buffer.write_data());
        REQUIRE(std::strncmp(buffer.read_data(), "abc", 3) == 0);

        old_cap = buffer.capacity();
        buffer.grow();
        REQUIRE(buffer.capacity() > old_cap);
        REQUIRE(buffer.read_size() == 3);
        REQUIRE(buffer.write_size() == buffer.capacity() - 3);
        REQUIRE(buffer.read_data() + 3 == buffer.write_data());
        REQUIRE(std::strncmp(buffer.read_data(), "abc", 3) == 0);
    }

    buffer.clear();
    REQUIRE(buffer.read_size() == 0);
    REQUIRE(buffer.write_size() == buffer.capacity());
    REQUIRE(buffer.read_data() == buffer.write_data());
}

TEST_CASE("_detail::buffer_builder::stable_iterator")
{
    lexy::_detail::buffer_builder<char> buffer;
    {
        auto input = "012346789";
        std::strcpy(buffer.write_data(), input); // NOLINT
        buffer.commit(std::strlen(input));
    }

    using iterator = decltype(buffer)::stable_iterator;

    auto iter = iterator(buffer, 0);
    CHECK(&*iter == buffer.read_data());

    SUBCASE("no grow") {}
    SUBCASE("grow")
    {
        buffer.grow();
    }

    auto end = iterator(buffer, buffer.read_size());
    CHECK(iter != end);

    ++iter;
    CHECK(iter != end);
    CHECK(&*iter == buffer.read_data() + 1);

    iter++;
    CHECK(iter != end);
    CHECK(&*iter == buffer.read_data() + 2);

    for (auto i = 3u; i < buffer.read_size(); ++i)
    {
        ++iter;
        CHECK(iter != end);
        CHECK(&*iter == buffer.read_data() + i);
    }

    ++iter;
    CHECK(iter == end);
}

