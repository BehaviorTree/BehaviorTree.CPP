// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/literal.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/case_folding.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/symbol.hpp>

TEST_CASE("dsl::lit_c")
{
    constexpr auto rule = dsl::lit_c<'a'>;
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(lexy::is_literal_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, LEXY_LIT("a")));
}

TEST_CASE("dsl::lit_b")
{
    constexpr auto rule = dsl::lit_b<'a', 'b', 'c'>;
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(lexy::is_literal_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("abc"));
    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd.status == test_result::success);
    CHECK(abcd.trace == test_trace().literal("abc"));

    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().error_token("a").expected_literal(0, "abc", 1).cancel());
    auto ad = LEXY_VERIFY("ad");
    CHECK(ad.status == test_result::fatal_error);
    CHECK(ad.trace == test_trace().error_token("a").expected_literal(0, "abc", 1).cancel());

    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());
    auto abd = LEXY_VERIFY("abd");
    CHECK(abd.status == test_result::fatal_error);
    CHECK(abd.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());

    auto ABC = LEXY_VERIFY("ABC");
    CHECK(ABC.status == test_result::fatal_error);
    CHECK(ABC.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto utf16 = LEXY_VERIFY(u"abc");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().literal("abc"));
}

TEST_CASE("dsl::lit")
{
    constexpr auto callback = token_callback;

    SUBCASE("ASCII")
    {
        constexpr auto rule = LEXY_LIT("abc");
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
        CHECK(equivalent_rules(rule, dsl::lit<"abc">));
#endif

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::success);
        CHECK(abcd.trace == test_trace().literal("abc"));

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().error_token("a").expected_literal(0, "abc", 1).cancel());
        auto ad = LEXY_VERIFY("ad");
        CHECK(ad.status == test_result::fatal_error);
        CHECK(ad.trace == test_trace().error_token("a").expected_literal(0, "abc", 1).cancel());

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());
        auto abd = LEXY_VERIFY("abd");
        CHECK(abd.status == test_result::fatal_error);
        CHECK(abd.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());

        auto ABC = LEXY_VERIFY("ABC");
        CHECK(ABC.status == test_result::fatal_error);
        CHECK(ABC.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto utf16 = LEXY_VERIFY(u"abc");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().literal("abc"));
    }
    SUBCASE("UTF-16, but only in ASCII")
    {
        constexpr auto rule = LEXY_LIT(u"abc");
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
        CHECK(equivalent_rules(rule, dsl::lit<u"abc">));
#endif

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));

        auto utf16 = LEXY_VERIFY(u"abc");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().literal("abc"));
    }
    SUBCASE("UTF-16, non ASCII")
    {
        constexpr auto rule = LEXY_LIT(u"äöü");
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
        CHECK(equivalent_rules(rule, dsl::lit<u"äöü">));
#endif

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "\\u00E4\\u00F6\\u00FC", 0).cancel());

        auto umlaute = LEXY_VERIFY(u"äöü");
        CHECK(umlaute.status == test_result::success);
        CHECK(umlaute.trace == test_trace().literal("\\u00E4\\u00F6\\u00FC"));
    }
}

TEST_CASE("dsl::lit_cp")
{
    // We're only testing UTF-16 inputs here for simplicity.
    // The actual logic is the code point encoding, which is tested elsewhere.

    constexpr auto callback = token_callback;

    SUBCASE("ASCII")
    {
        constexpr auto rule = dsl::lit_cp<'a'>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto ok = LEXY_VERIFY(u"a");
        CHECK(ok.status == test_result::success);
        CHECK(ok.trace == test_trace().literal("a"));

        auto not_ascii = LEXY_VERIFY(u"b");
        CHECK(not_ascii.status == test_result::fatal_error);
        CHECK(not_ascii.trace == test_trace().expected_literal(0, "a", 0).cancel());
        auto not_bmp = LEXY_VERIFY(u"ä");
        CHECK(not_bmp.status == test_result::fatal_error);
        CHECK(not_bmp.trace == test_trace().expected_literal(0, "a", 0).cancel());
        auto not_multi = LEXY_VERIFY(u"😂");
        CHECK(not_multi.status == test_result::fatal_error);
        CHECK(not_multi.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto twice = LEXY_VERIFY(u"aa");
        CHECK(twice.status == test_result::success);
        CHECK(twice.trace == test_trace().literal("a"));

        auto ascii = LEXY_VERIFY(lexy::ascii_encoding{}, "a");
        CHECK(ascii.status == test_result::success);
        CHECK(ascii.trace == test_trace().literal("a"));
    }
    SUBCASE("BMP")
    {
        constexpr auto rule = dsl::lit_cp<0x00E4>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "\\u00E4", 0).cancel());

        auto ok = LEXY_VERIFY(u"ä");
        CHECK(ok.status == test_result::success);
        CHECK(ok.trace == test_trace().literal("\\u00E4"));

        auto not_ascii = LEXY_VERIFY(u"a");
        CHECK(not_ascii.status == test_result::fatal_error);
        CHECK(not_ascii.trace == test_trace().expected_literal(0, "\\u00E4", 0).cancel());
        auto not_bmp = LEXY_VERIFY(u"ü");
        CHECK(not_bmp.status == test_result::fatal_error);
        CHECK(not_bmp.trace == test_trace().expected_literal(0, "\\u00E4", 0).cancel());
        auto not_multi = LEXY_VERIFY(u"🙂");
        CHECK(not_multi.status == test_result::fatal_error);
        CHECK(not_multi.trace == test_trace().expected_literal(0, "\\u00E4", 0).cancel());

        auto twice = LEXY_VERIFY(u"ää");
        CHECK(twice.status == test_result::success);
        CHECK(twice.trace == test_trace().literal("\\u00E4"));
    }
    SUBCASE("multi")
    {
        constexpr auto rule = dsl::lit_cp<0x1F642>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "\\U0001F642", 0).cancel());

        auto ok = LEXY_VERIFY(u"🙂");
        CHECK(ok.status == test_result::success);
        CHECK(ok.trace == test_trace().literal("\\U0001F642"));

        auto not_ascii = LEXY_VERIFY(u"a");
        CHECK(not_ascii.status == test_result::fatal_error);
        CHECK(not_ascii.trace == test_trace().expected_literal(0, "\\U0001F642", 0).cancel());
        auto not_bmp = LEXY_VERIFY(u"ü");
        CHECK(not_bmp.status == test_result::fatal_error);
        CHECK(not_bmp.trace == test_trace().expected_literal(0, "\\U0001F642", 0).cancel());
        auto not_multi = LEXY_VERIFY(u"😂"); // note: same leading surrogate
        CHECK(not_multi.status == test_result::fatal_error);
        CHECK(not_multi.trace
              == test_trace()
                     .error_token("\\xD8\\x3D")
                     .expected_literal(0, "\\U0001F642", 1)
                     .cancel());

        auto twice = LEXY_VERIFY(u"🙂🙂");
        CHECK(twice.status == test_result::success);
        CHECK(twice.trace == test_trace().literal("\\U0001F642"));
    }

    SUBCASE("sequence")
    {
        constexpr auto rule = dsl::lit_cp<'a', 0x00E4, 0x1F642>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a\\u00E4\\U0001F642", 0).cancel());

        auto ok = LEXY_VERIFY(u"aä🙂");
        CHECK(ok.status == test_result::success);
        CHECK(ok.trace == test_trace().literal("a\\u00E4\\U0001F642"));

        auto partial_cp = LEXY_VERIFY(u"aä");
        CHECK(partial_cp.status == test_result::fatal_error);
        CHECK(partial_cp.trace
              == test_trace()
                     .error_token("a\\u00E4")
                     .expected_literal(0, "a\\u00E4\\U0001F642", 2)
                     .cancel());
        auto partial_cu = LEXY_VERIFY(u"aä\U0001F643");
        CHECK(partial_cu.status == test_result::fatal_error);
        CHECK(partial_cu.trace
              == test_trace()
                     .error_token("a\\u00E4\\xD8\\x3D")
                     .expected_literal(0, "a\\u00E4\\U0001F642", 3)
                     .cancel());
    }
}

TEST_CASE("dsl::literal_set")
{
    constexpr auto callback = token_callback;

    SUBCASE("empty")
    {
        constexpr auto rule = dsl::literal_set();
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().error(0, 0, "expected literal set").cancel());
    }
    SUBCASE("single")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc"));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
    }

    SUBCASE("disjoint")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc"), LEXY_LIT("123"), LEXY_LIT("hello"));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto _123 = LEXY_VERIFY("123");
        CHECK(_123.status == test_result::success);
        CHECK(_123.trace == test_trace().literal("123"));
        auto hello = LEXY_VERIFY("hello");
        CHECK(hello.status == test_result::success);
        CHECK(hello.trace == test_trace().literal("hello"));

        auto utf16 = LEXY_VERIFY(u"abc");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().literal("abc"));
    }
    SUBCASE("common prefix")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc"), LEXY_LIT("abd"));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto abd = LEXY_VERIFY("abd");
        CHECK(abd.status == test_result::success);
        CHECK(abd.trace == test_trace().literal("abd"));
    }
    SUBCASE("substring")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc"), LEXY_LIT("ab"));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().literal("ab"));
    }
    SUBCASE("identical")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc"), LEXY_LIT("abc"));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
    }

    SUBCASE("lit_b")
    {
        constexpr auto rule
            = dsl::literal_set(dsl::lit_b<'a', 'b', 'c'>, dsl::lit_b<'a', 'b', '\0'>);
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto ab0 = LEXY_VERIFY("ab\0", 3);
        CHECK(ab0.status == test_result::success);
        CHECK(ab0.trace == test_trace().literal("ab\\0"));
    }
    SUBCASE("lit_cp")
    {
        // These share a common prefix in UTF-8 (0xC3)
        constexpr auto rule
            = dsl::literal_set(dsl::lit_cp<U'ä'>, dsl::lit_cp<U'ö'>, dsl::lit_cp<U'ü'>);
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto a_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("ä"));
        CHECK(a_utf8.status == test_result::success);
        CHECK(a_utf8.trace == test_trace().literal("\\u00E4"));
        auto o_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("ö"));
        CHECK(o_utf8.status == test_result::success);
        CHECK(o_utf8.trace == test_trace().literal("\\u00F6"));
        auto u_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("ü"));
        CHECK(u_utf8.status == test_result::success);
        CHECK(u_utf8.trace == test_trace().literal("\\u00FC"));

        auto a_utf16 = LEXY_VERIFY(lexy::utf16_encoding{}, u"ä");
        CHECK(a_utf16.status == test_result::success);
        CHECK(a_utf16.trace == test_trace().literal("\\u00E4"));
        auto o_utf16 = LEXY_VERIFY(lexy::utf16_encoding{}, u"ö");
        CHECK(o_utf16.status == test_result::success);
        CHECK(o_utf16.trace == test_trace().literal("\\u00F6"));
        auto u_utf16 = LEXY_VERIFY(lexy::utf16_encoding{}, u"ü");
        CHECK(u_utf16.status == test_result::success);
        CHECK(u_utf16.trace == test_trace().literal("\\u00FC"));
    }

    SUBCASE("keyword")
    {
        constexpr auto id1 = dsl::identifier(dsl::ascii::alpha);
        constexpr auto id2 = dsl::identifier(dsl::ascii::alpha, dsl::ascii::digit);
        constexpr auto rule
            = dsl::literal_set(LEXY_LIT("ab"), LEXY_KEYWORD("abc", id1), LEXY_KEYWORD("a12", id2));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().literal("ab"));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::success);
        CHECK(abcd.trace == test_trace().literal("ab"));

        auto a12 = LEXY_VERIFY("a12");
        CHECK(a12.status == test_result::success);
        CHECK(a12.trace == test_trace().literal("a12"));
        auto a123 = LEXY_VERIFY("a123");
        CHECK(a123.status == test_result::fatal_error);
        CHECK(a123.trace == test_trace().error(0, 0, "expected literal set").cancel());
    }

    SUBCASE("case folding")
    {
        constexpr auto rule = dsl::literal_set(dsl::ascii::case_folding(LEXY_LIT("abc")),
                                               LEXY_LIT("123"), LEXY_LIT("hello"));
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto ABC = LEXY_VERIFY("ABC");
        CHECK(ABC.status == test_result::success);
        CHECK(ABC.trace == test_trace().literal("ABC"));
        auto _123 = LEXY_VERIFY("123");
        CHECK(_123.status == test_result::success);
        CHECK(_123.trace == test_trace().literal("123"));
        auto hello = LEXY_VERIFY("hello");
        CHECK(hello.status == test_result::success);
        CHECK(hello.trace == test_trace().literal("hello"));
        auto HellO = LEXY_VERIFY("HellO");
        CHECK(HellO.status == test_result::success);
        CHECK(HellO.trace == test_trace().literal("HellO"));
    }
}

TEST_CASE("LEXY_LITERAL_SET")
{
    constexpr auto rule = LEXY_LITERAL_SET(LEXY_LIT("abc"), LEXY_LIT("abd"));
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(lexy::is_literal_set_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("abc"));
    auto abd = LEXY_VERIFY("abd");
    CHECK(abd.status == test_result::success);
    CHECK(abd.trace == test_trace().literal("abd"));
}

namespace
{
enum class token_kind
{
    my_kind,
};

[[maybe_unused]] constexpr const char* token_kind_name(token_kind)
{
    return "my_kind";
}
} // namespace

TEST_CASE("dsl::literal_set() .kind and .error")
{
    struct my_error
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "my_error";
        }
    };

    SUBCASE(".kind")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc")).kind<token_kind::my_kind>;
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto alpha = LEXY_VERIFY("abc");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("my_kind", "abc"));
    }
    SUBCASE(".error")
    {
        constexpr auto rule = dsl::literal_set(LEXY_LIT("abc")).error<my_error>;
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

        auto alpha = LEXY_VERIFY("abc");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().literal("abc"));
    }
    SUBCASE(".kind.error")
    {
        constexpr auto rule
            = dsl::literal_set(LEXY_LIT("abc")).kind<token_kind::my_kind>.error<my_error>;
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

        auto alpha = LEXY_VERIFY("abc");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("my_kind", "abc"));
    }
    SUBCASE(".error.kind")
    {
        constexpr auto rule
            = dsl::literal_set(LEXY_LIT("abc")).error<my_error>.kind<token_kind::my_kind>;
        CHECK(lexy::is_literal_set_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

        auto alpha = LEXY_VERIFY("abc");
        CHECK(alpha.status == test_result::success);
        CHECK(alpha.trace == test_trace().token("my_kind", "abc"));
    }
}

TEST_CASE("dsl::literal_set() operator/")
{
    constexpr auto with_literals = dsl::literal_set() / LEXY_LIT("abc") / dsl::lit_c<'d'>;
    CHECK(equivalent_rules(with_literals, dsl::literal_set(LEXY_LIT("abc"), dsl::lit_c<'d'>)));

    constexpr auto with_sets = dsl::literal_set()
                               / dsl::literal_set(dsl::lit_c<'a'>, dsl::lit_c<'b'>)
                               / dsl::literal_set(dsl::lit_c<'c'>, dsl::lit_c<'d'>);
    CHECK(equivalent_rules(with_sets, dsl::literal_set(dsl::lit_c<'a'>, dsl::lit_c<'b'>,
                                                       dsl::lit_c<'c'>, dsl::lit_c<'d'>)));

    constexpr auto with_erased_sets = dsl::literal_set()
                                      / LEXY_LITERAL_SET(dsl::lit_c<'a'>, dsl::lit_c<'b'>)
                                      / LEXY_LITERAL_SET(dsl::lit_c<'c'>, dsl::lit_c<'d'>);
    CHECK(equivalent_rules(with_erased_sets, dsl::literal_set(dsl::lit_c<'a'>, dsl::lit_c<'b'>,
                                                              dsl::lit_c<'c'>, dsl::lit_c<'d'>)));

    constexpr auto set_a        = LEXY_LITERAL_SET(dsl::lit_c<'a'>);
    constexpr auto keep_erasure = dsl::literal_set() / set_a;
    CHECK(equivalent_rules(keep_erasure, set_a));
}

TEST_CASE("dsl::literal_set() from symbol table")
{
    constexpr auto basic = dsl::literal_set(
        lexy::symbol_table<int>.map<'a'>(0).map<LEXY_SYMBOL("b")>(1).map(LEXY_LIT("c"), 2));
    CHECK(equivalent_rules(basic, dsl::literal_set(LEXY_LIT("a"), LEXY_LIT("b"), LEXY_LIT("c"))));

    constexpr auto case_folding = dsl::literal_set(
        lexy::symbol_table<int>.case_folding(dsl::ascii::case_folding).map<'a'>(0).map<LEXY_SYMBOL("b")>(1).map(LEXY_LIT("c"), 2));
    CHECK(
        equivalent_rules(case_folding, dsl::literal_set(dsl::ascii::case_folding(LEXY_LIT("a")),
                                                        dsl::ascii::case_folding(LEXY_LIT("b")),
                                                        dsl::ascii::case_folding(LEXY_LIT("c")))));
}

