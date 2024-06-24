// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/option.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <optional>

namespace
{
template <typename T>
constexpr auto is_optional_like = lexy::_detail::is_detected<lexy::_detect_optional_like, T>;
} // namespace

TEST_CASE("lexy::nullopt")
{
    CHECK(is_optional_like<std::optional<int>>);
    CHECK(!std::optional<int>(lexy::nullopt{}).has_value());

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#endif
    // This code triggers GCC conversion warning as this isn't something we actually want,
    // but there is no way of fixing it.
    CHECK(is_optional_like<std::optional<std::optional<int>>>);
    CHECK(std::optional<std::optional<int>>(lexy::nullopt{}).has_value());
    CHECK(!std::optional<std::optional<int>>(lexy::nullopt{})->has_value());
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

    CHECK(is_optional_like<int*>);
    CHECK(static_cast<int*>(lexy::nullopt{}) == nullptr);

    CHECK(!is_optional_like<int>);
}

TEST_CASE("dsl::nullopt")
{
    constexpr auto rule = dsl::nullopt;
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char*, lexy::nullopt) { return 0; };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.value == 0);
    CHECK(abc.trace == test_trace());
}

TEST_CASE("dsl::opt()")
{
    constexpr auto rule = dsl::opt(dsl::capture(LEXY_LIT("ab")) >> dsl::capture(LEXY_LIT("cd")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback //
        = lexy::callback<int>([](const char*, lexy::nullopt) { return 0; },
                              [](const char* begin, lexy::string_lexeme<> ab,
                                 lexy::string_lexeme<> cd) {
                                  CHECK(ab.size() == 2);
                                  CHECK(ab.begin() == begin);
                                  CHECK(ab[0] == 'a');
                                  CHECK(ab[1] == 'b');

                                  CHECK(cd.size() == 2);
                                  CHECK(cd.begin() == begin + 2);
                                  CHECK(cd[0] == 'c');
                                  CHECK(cd[1] == 'd');

                                  return 1;
                              });

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());
    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::success);
    CHECK(a.value == 0);
    CHECK(a.trace == test_trace());

    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().literal("ab").expected_literal(2, "cd", 0).cancel());
    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace
          == test_trace().literal("ab").error_token("c").expected_literal(2, "cd", 1).cancel());

    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd.status == test_result::success);
    CHECK(abcd.value == 1);
    CHECK(abcd.trace == test_trace().literal("ab").literal("cd"));
    auto abcde = LEXY_VERIFY("abcde");
    CHECK(abcde.status == test_result::success);
    CHECK(abcde.value == 1);
    CHECK(abcde.trace == test_trace().literal("ab").literal("cd"));
}

TEST_CASE("dsl::opt(unconditional)")
{
    constexpr auto rule = dsl::opt(dsl::else_ >> dsl::capture(LEXY_LIT("cd")));
    CHECK(lexy::is_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::else_ >> dsl::capture(LEXY_LIT("cd"))));
}

