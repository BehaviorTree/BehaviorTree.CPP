// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/terminator.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/position.hpp>

TEST_CASE("dsl::terminator()")
{
    constexpr auto term = dsl::terminator(LEXY_LIT("!!!") >> dsl::position);

    CHECK(equivalent_rules(term.terminator(), LEXY_LIT("!!!") >> dsl::position));
    CHECK(equivalent_rules(term.recovery_rule(), dsl::recover(term.terminator())));

    CHECK(equivalent_rules(term.limit(dsl::lit_c<';'>).recovery_rule(),
                           dsl::recover(term.terminator()).limit(dsl::lit_c<';'>)));
    CHECK(equivalent_rules(term.limit(dsl::lit_c<';'>).limit(dsl::lit_c<'.'>),
                           term.limit(dsl::lit_c<';'>, dsl::lit_c<'.'>)));

    SUBCASE("operator() rule")
    {
        constexpr auto rule = term(dsl::position);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(equivalent_rules(rule, dsl::position + term.terminator()));
    }
    SUBCASE("operator() branch")
    {
        constexpr auto rule = term(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(equivalent_rules(rule, LEXY_LIT("abc") >> term.terminator()));
    }

    SUBCASE(".try_()")
    {
        constexpr auto rule = term.limit(dsl::lit_c<';'>).try_(LEXY_LIT("abc") + dsl::position);
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = lexy::callback<int>([](const char*, const char*) { return 0; },
                                  [](const char*, const char*, const char*) { return 1; });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "abc", 0).recovery().cancel().cancel());

        auto null = LEXY_VERIFY("!!!");
        CHECK(null.status == test_result::recovered_error);
        CHECK(null.value == 0);
        CHECK(null.trace
              == test_trace()
                     .expected_literal(0, "abc", 0)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());

        auto abc = LEXY_VERIFY("abc!!!");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").position().literal("!!!").position());

        auto ab = LEXY_VERIFY("ab!!!");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.value == 0);
        CHECK(ab.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());

        auto unterminated = LEXY_VERIFY("abc");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("abc").position().expected_literal(3, "!!!", 0).cancel());
        auto partial_terminator = LEXY_VERIFY("abc!");
        CHECK(partial_terminator.status == test_result::fatal_error);
        CHECK(partial_terminator.trace
              == test_trace()
                     .literal("abc")
                     .position()
                     .error_token("!")
                     .expected_literal(3, "!!!", 1)
                     .cancel());
        auto other_terminator = LEXY_VERIFY("abc???");
        CHECK(other_terminator.status == test_result::fatal_error);
        CHECK(other_terminator.trace
              == test_trace().literal("abc").position().expected_literal(3, "!!!", 0).cancel());
        auto later_terminator = LEXY_VERIFY("abcdef!!!");
        CHECK(later_terminator.status == test_result::fatal_error);
        CHECK(later_terminator.trace
              == test_trace().literal("abc").position().expected_literal(3, "!!!", 0).cancel());

        auto limited = LEXY_VERIFY("abde;abc!!!");
        CHECK(limited.status == test_result::fatal_error);
        CHECK(limited.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .error_token("de")
                     .cancel()
                     .cancel());
    }

    SUBCASE(".opt()")
    {
        constexpr auto rule = term.limit(dsl::lit_c<';'>).opt(dsl::capture(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = lexy::callback<int>([](const char*, lexy::nullopt, const char*) { return 0; },
                                  [](const char* begin, lexy::string_lexeme<> lex, const char*) {
                                      CHECK(lex.begin() == begin);
                                      CHECK(lex.size() == 3);

                                      CHECK(lex[0] == 'a');
                                      CHECK(lex[1] == 'b');
                                      CHECK(lex[2] == 'c');

                                      return 1;
                                  },
                                  [](const char*, const char*) { return 2; });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "abc", 0).recovery().cancel().cancel());

        auto null = LEXY_VERIFY("!!!");
        CHECK(null.status == test_result::success);
        CHECK(null.value == 0);
        CHECK(null.trace == test_trace().literal("!!!").position());

        auto abc = LEXY_VERIFY("abc!!!");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").literal("!!!").position());

        auto ab = LEXY_VERIFY("ab!!!");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.value == 2);
        CHECK(ab.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());

        auto unterminated = LEXY_VERIFY("abc");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace().literal("abc").expected_literal(3, "!!!", 0).cancel());

        auto limited = LEXY_VERIFY("abde;abc!!!");
        CHECK(limited.status == test_result::fatal_error);
        CHECK(limited.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .error_token("de")
                     .cancel()
                     .cancel());
    }

    SUBCASE(".list(branch)")
    {
        constexpr auto rule
            = term.limit(dsl::lit_c<';'>).list(LEXY_LIT("ab") >> dsl::capture(LEXY_LIT("c")));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = [](const char*, std::size_t count, const char*) { return int(count); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto zero = LEXY_VERIFY("!!!");
        CHECK(zero.status == test_result::recovered_error);
        CHECK(zero.trace
              == test_trace()
                     .expected_literal(0, "ab", 0)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());
        auto one = LEXY_VERIFY("abc!!!");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("ab").literal("c").literal("!!!").position());
        auto two = LEXY_VERIFY("abcabc!!!");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());
        auto three = LEXY_VERIFY("abcabcabc!!!");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());

        auto recover_item = LEXY_VERIFY("abcaabc!!!");
        CHECK(recover_item.status == test_result::recovered_error);
        CHECK(recover_item.value == 2);
        CHECK(recover_item.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .error_token("a")
                     .expected_literal(3, "ab", 1)
                     .recovery().finish()
                     .literal("ab").literal("c")
                     .literal("!!!").position());
        // clang-format on
        auto recover_item_failed = LEXY_VERIFY("abcaababc!!!");
        CHECK(recover_item_failed.status == test_result::recovered_error);
        CHECK(recover_item_failed.value == 2);
        CHECK(recover_item_failed.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .error_token("a")
                     .expected_literal(3, "ab", 1)
                     .recovery().finish()
                     .literal("ab").expected_literal(6, "c", 0)
                     .recovery().finish()
                     .literal("ab").literal("c")
                     .literal("!!!").position());
        // clang-format on
        auto recover_term = LEXY_VERIFY("abcabd!!!");
        CHECK(recover_term.status == test_result::recovered_error);
        CHECK(recover_term.value == 1);
        CHECK(recover_term.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal("ab").expected_literal(5, "c", 0)
                     .recovery()
                          .error_token("d")
                         .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_limit = LEXY_VERIFY("abcabd;abc!!!");
        CHECK(recover_limit.status == test_result::fatal_error);
        CHECK(recover_limit.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal("ab").expected_literal(5, "c", 0)
                     .recovery()
                          .error_token("d")
                         .cancel()
                     .cancel());
        // clang-format on
    }
    SUBCASE(".list(rule)")
    {
        constexpr auto rule
            = term.limit(dsl::lit_c<';'>).list(LEXY_LIT("ab") + dsl::capture(LEXY_LIT("c")));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = [](const char*, std::size_t count, const char*) { return int(count); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto zero = LEXY_VERIFY("!!!");
        CHECK(zero.status == test_result::recovered_error);
        CHECK(zero.trace
              == test_trace()
                     .expected_literal(0, "ab", 0)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());
        auto one = LEXY_VERIFY("abc!!!");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("ab").literal("c").literal("!!!").position());
        auto two = LEXY_VERIFY("abcabc!!!");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());
        auto three = LEXY_VERIFY("abcabcabc!!!");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());

        auto recover_item = LEXY_VERIFY("abcaabc!!!"); // can't actually recover at the next item
        CHECK(recover_item.status == test_result::recovered_error);
        CHECK(recover_item.value == 1);
        CHECK(recover_item.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .error_token("a")
                     .expected_literal(3, "ab", 1)
                     .recovery()
                         .error_token("abc")
                         .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_term = LEXY_VERIFY("abcabd!!!");
        CHECK(recover_term.status == test_result::recovered_error);
        CHECK(recover_term.value == 1);
        CHECK(recover_term.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal("ab").expected_literal(5, "c", 0)
                     .recovery()
                        .error_token("d")
                        .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_limit = LEXY_VERIFY("abcabd;abc!!!");
        CHECK(recover_limit.status == test_result::fatal_error);
        CHECK(recover_limit.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal("ab").expected_literal(5, "c", 0)
                     .recovery()
                          .error_token("d")
                         .cancel()
                     .cancel());
        // clang-format on
    }
    SUBCASE(".list(branch, sep)")
    {
        constexpr auto rule = term.limit(dsl::lit_c<';'>)
                                  .list(LEXY_LIT("ab") >> dsl::capture(LEXY_LIT("c")),
                                        dsl::sep((dsl::lit_c<','>) >> dsl::lit_c<','>));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = [](const char*, std::size_t count, const char*) { return int(count); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto zero = LEXY_VERIFY("!!!");
        CHECK(zero.status == test_result::recovered_error);
        CHECK(zero.trace
              == test_trace()
                     .expected_literal(0, "ab", 0)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());
        auto one = LEXY_VERIFY("abc!!!");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("ab").literal("c").literal("!!!").position());
        auto two = LEXY_VERIFY("abc,,abc!!!");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());
        auto three = LEXY_VERIFY("abc,,abc,,abc!!!");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());

        auto trailing = LEXY_VERIFY("abc,,abc,,!!!");
        CHECK(trailing.status == test_result::recovered_error);
        CHECK(trailing.value == 2);
        CHECK(trailing.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .error(8, 10, "unexpected trailing separator")
                     .literal("!!!")
                     .position());

        auto no_sep = LEXY_VERIFY("abcabc!!!");
        CHECK(no_sep.status == test_result::recovered_error);
        CHECK(no_sep.value == 2);
        CHECK(no_sep.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .expected_literal(3, ",", 0)
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());
        auto no_sep_no_item = LEXY_VERIFY("abcd!!!");
        CHECK(no_sep_no_item.status == test_result::recovered_error);
        CHECK(no_sep_no_item.value == 1);
        CHECK(no_sep_no_item.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .expected_literal(3, ",", 0)
                     .recovery()
                     .error_token("d")
                     .finish()
                     .literal("!!!")
                     .position());
        auto no_sep_partial_item = LEXY_VERIFY("abcab!!!");
        CHECK(no_sep_partial_item.status == test_result::recovered_error);
        CHECK(no_sep_partial_item.value == 1);
        CHECK(no_sep_partial_item.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .expected_literal(3, ",", 0)
                     .literal("ab")
                     .expected_literal(5, "c", 0)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());

        auto partial_sep = LEXY_VERIFY("abc,abc!!!");
        CHECK(partial_sep.status == test_result::recovered_error);
        CHECK(partial_sep.value == 1);
        CHECK(partial_sep.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .expected_literal(4, ",", 0)
                     .recovery()
                     .error_token("abc")
                     .finish()
                     .literal("!!!")
                     .position());

        auto recover_sep = LEXY_VERIFY("abc,,a,,abc!!!");
        CHECK(recover_sep.status == test_result::recovered_error);
        CHECK(recover_sep.value == 2);
        CHECK(recover_sep.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .error_token("a")
                     .expected_literal(5, "ab", 1)
                     .recovery().finish()
                     .literal(",").literal(",")
                     .literal("ab").literal("c")
                     .literal("!!!").position());
        // clang-format on
        auto recover_sep_failed = LEXY_VERIFY("abc,,a,abc!!!");
        CHECK(recover_sep_failed.status == test_result::recovered_error);
        CHECK(recover_sep_failed.value == 1);
        CHECK(recover_sep_failed.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .error_token("a")
                     .expected_literal(5, "ab", 1)
                     .recovery().finish()
                     .literal(",").expected_literal(7, ",", 0)
                     .recovery()
                         .error_token("abc")
                         .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_term = LEXY_VERIFY("abc,,abd!!!");
        CHECK(recover_term.status == test_result::recovered_error);
        CHECK(recover_term.value == 1);
        CHECK(recover_term.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .literal("ab").expected_literal(7, "c", 0)
                     .recovery()
                          .error_token("d")
                         .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_limit = LEXY_VERIFY("abc,,abd;abc!!!");
        CHECK(recover_limit.status == test_result::fatal_error);
        CHECK(recover_limit.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .literal("ab").expected_literal(7, "c", 0)
                     .recovery()
                          .error_token("d")
                         .cancel()
                     .cancel());
        // clang-format on
    }
    SUBCASE(".list(rule, sep)")
    {
        constexpr auto rule = term.limit(dsl::lit_c<';'>)
                                  .list(LEXY_LIT("ab") + dsl::capture(LEXY_LIT("c")),
                                        dsl::sep((dsl::lit_c<','>) >> dsl::lit_c<','>));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = [](const char*, std::size_t count, const char*) { return int(count); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto zero = LEXY_VERIFY("!!!");
        CHECK(zero.status == test_result::recovered_error);
        CHECK(zero.trace
              == test_trace()
                     .expected_literal(0, "ab", 0)
                     .recovery()
                     .finish()
                     .literal("!!!")
                     .position());
        auto one = LEXY_VERIFY("abc!!!");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("ab").literal("c").literal("!!!").position());
        auto two = LEXY_VERIFY("abc,,abc!!!");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());
        auto three = LEXY_VERIFY("abc,,abc,,abc!!!");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());

        auto trailing = LEXY_VERIFY("abc,,abc,,!!!");
        CHECK(trailing.status == test_result::recovered_error);
        CHECK(trailing.value == 2);
        CHECK(trailing.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .error(8, 10, "unexpected trailing separator")
                     .literal("!!!")
                     .position());

        auto no_sep = LEXY_VERIFY("abcabc!!!");
        CHECK(no_sep.status == test_result::recovered_error);
        CHECK(no_sep.value == 1);
        CHECK(no_sep.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .expected_literal(3, ",", 0)
                     .recovery()
                     .error_token("abc")
                     .finish()
                     .literal("!!!")
                     .position());
        auto no_sep_no_item = LEXY_VERIFY("abcd!!!");
        CHECK(no_sep_no_item.status == test_result::recovered_error);
        CHECK(no_sep_no_item.value == 1);
        CHECK(no_sep_no_item.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .expected_literal(3, ",", 0)
                     .recovery()
                     .error_token("d")
                     .finish()
                     .literal("!!!")
                     .position());
        auto no_sep_partial_item = LEXY_VERIFY("abcab!!!");
        CHECK(no_sep_partial_item.status == test_result::recovered_error);
        CHECK(no_sep_partial_item.value == 1);
        CHECK(no_sep_partial_item.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .expected_literal(3, ",", 0)
                     .recovery()
                     .error_token("ab")
                     .finish()
                     .literal("!!!")
                     .position());

        auto partial_sep = LEXY_VERIFY("abc,abc!!!");
        CHECK(partial_sep.status == test_result::recovered_error);
        CHECK(partial_sep.value == 1);
        CHECK(partial_sep.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .expected_literal(4, ",", 0)
                     .recovery()
                     .error_token("abc")
                     .finish()
                     .literal("!!!")
                     .position());

        auto recover_sep = LEXY_VERIFY("abc,,a,,abc!!!");
        CHECK(recover_sep.status == test_result::recovered_error);
        CHECK(recover_sep.value == 2);
        CHECK(recover_sep.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .error_token("a")
                     .expected_literal(5, "ab", 1)
                     .recovery().finish()
                     .literal(",").literal(",")
                     .literal("ab").literal("c")
                     .literal("!!!").position());
        // clang-format on
        auto recover_sep_failed = LEXY_VERIFY("abc,,a,abc!!!");
        CHECK(recover_sep_failed.status == test_result::recovered_error);
        CHECK(recover_sep_failed.value == 1);
        CHECK(recover_sep_failed.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .error_token("a")
                     .expected_literal(5, "ab", 1)
                     .recovery().finish()
                     .literal(",").expected_literal(7, ",", 0)
                     .recovery()
                         .error_token("abc")
                         .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_term = LEXY_VERIFY("abc,,abd!!!");
        CHECK(recover_term.status == test_result::recovered_error);
        CHECK(recover_term.value == 1);
        CHECK(recover_term.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .literal("ab").expected_literal(7, "c", 0)
                     .recovery()
                          .error_token("d")
                         .finish()
                     .literal("!!!").position());
        // clang-format on
        auto recover_limit = LEXY_VERIFY("abc,,abd;abc!!!");
        CHECK(recover_limit.status == test_result::fatal_error);
        CHECK(recover_limit.trace
              // clang-format off
              == test_trace()
                     .literal("ab").literal("c")
                     .literal(",").literal(",")
                     .literal("ab").expected_literal(7, "c", 0)
                     .recovery()
                          .error_token("d")
                         .cancel()
                     .cancel());
        // clang-format on
    }
    SUBCASE(".list(branch, trailing_sep)")
    {
        constexpr auto rule = term.limit(dsl::lit_c<';'>)
                                  .list(LEXY_LIT("ab") >> dsl::capture(LEXY_LIT("c")),
                                        dsl::trailing_sep((dsl::lit_c<','>) >> dsl::lit_c<','>));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = [](const char*, std::size_t count, const char*) { return int(count); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto trailing = LEXY_VERIFY("abc,,abc,,!!!");
        CHECK(trailing.status == test_result::success);
        CHECK(trailing.value == 2);
        CHECK(trailing.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal(",")
                     .literal("!!!")
                     .position());
    }

    // Only simple checks necessary, it shares implementations between list and opt.
    SUBCASE(".opt_list(rule)")
    {
        constexpr auto rule
            = term.limit(dsl::lit_c<';'>).opt_list(LEXY_LIT("ab") >> dsl::capture(LEXY_LIT("c")));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = lexy::callback<int>([](const char*, lexy::nullopt, const char*) { return 0; },
                                  [](const char*, std::size_t count, const char*) {
                                      return int(count);
                                  });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto zero = LEXY_VERIFY("!!!");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("!!!").position());
        auto one = LEXY_VERIFY("abc!!!");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("ab").literal("c").literal("!!!").position());
        auto three = LEXY_VERIFY("abcabcabc!!!");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());

        auto recover = LEXY_VERIFY("abd!!!");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 0);
        CHECK(recover.trace
              == test_trace()
                     .literal("ab")
                     .expected_literal(2, "c", 0)
                     .recovery()
                     .error_token("d")
                     .finish()
                     .literal("!!!")
                     .position());
    }
    SUBCASE(".opt_list(rule, sep)")
    {
        constexpr auto rule = term.limit(dsl::lit_c<';'>)
                                  .opt_list(LEXY_LIT("ab") >> dsl::capture(LEXY_LIT("c")),
                                            dsl::sep(dsl::lit_c<','>));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = lexy::callback<int>([](const char*, lexy::nullopt, const char*) { return 0; },
                                  [](const char*, std::size_t count, const char*) {
                                      return int(count);
                                  });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto zero = LEXY_VERIFY("!!!");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("!!!").position());
        auto one = LEXY_VERIFY("abc!!!");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().literal("ab").literal("c").literal("!!!").position());
        auto three = LEXY_VERIFY("abc,abc,abc!!!");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal(",")
                     .literal("ab")
                     .literal("c")
                     .literal("!!!")
                     .position());

        auto recover = LEXY_VERIFY("abd!!!");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 0);
        CHECK(recover.trace
              == test_trace()
                     .literal("ab")
                     .expected_literal(2, "c", 0)
                     .recovery()
                     .error_token("d")
                     .finish()
                     .literal("!!!")
                     .position());
    }
}

