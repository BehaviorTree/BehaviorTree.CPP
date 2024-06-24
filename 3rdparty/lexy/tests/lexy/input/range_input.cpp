// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/range_input.hpp>

#include <doctest/doctest.h>

namespace
{
struct test_iterator
{
    int count;

    // No initialization in default ctor.

    char operator*() const
    {
        return 'a';
    }

    test_iterator& operator++()
    {
        --count;
        return *this;
    }

    [[maybe_unused]] friend bool operator==(test_iterator lhs, test_iterator rhs)
    {
        return lhs.count == rhs.count;
    }
    [[maybe_unused]] friend bool operator!=(test_iterator lhs, test_iterator rhs)
    {
        return lhs.count != rhs.count;
    }
};

struct test_sentinel
{
    [[maybe_unused]] friend bool operator==(test_iterator iter, test_sentinel)
    {
        return iter.count == 0;
    }
    [[maybe_unused]] friend bool operator!=(test_iterator iter, test_sentinel)
    {
        return iter.count != 0;
    }
    [[maybe_unused]] friend bool operator==(test_sentinel, test_iterator iter)
    {
        return iter.count == 0;
    }
    [[maybe_unused]] friend bool operator!=(test_sentinel, test_iterator iter)
    {
        return iter.count != 0;
    }
};
} // namespace

TEST_CASE("range_input")
{
    lexy::range_input<lexy::default_encoding, test_iterator, test_sentinel> input;
    CHECK(sizeof(input) == (LEXY_HAS_EMPTY_MEMBER ? sizeof(int) : 2 * sizeof(int)));

    CHECK(input.reader().position().count == 0);
    CHECK(input.reader().peek() == lexy::default_encoding::eof());

    input = lexy::range_input(test_iterator{3}, test_sentinel{});
    CHECK(input.begin() == test_iterator{3});
    CHECK(input.end() == test_iterator{0});

    auto reader = input.reader();
    CHECK(reader.position().count == 3);
    CHECK(reader.peek() == 'a');

    reader.bump();
    CHECK(reader.position().count == 2);
    CHECK(reader.peek() == 'a');

    reader.bump();
    CHECK(reader.position().count == 1);
    CHECK(reader.peek() == 'a');

    reader.bump();
    CHECK(reader.position().count == 0);
    CHECK(reader.peek() == lexy::default_encoding::eof());
}

