#include <optional>
#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct flag
{
    std::optional<std::string> key;
    std::string                value;
};

struct production
{
    struct flag_key
    {
        static constexpr auto rule  = dsl::identifier(dsl::ascii::alpha);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    struct flag_value
    {
        static constexpr auto rule  = dsl::identifier(dsl::ascii::alnum);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    static constexpr auto whitespace = dsl::ascii::blank; // no newline

    static constexpr auto rule = [] {
        // key = value
        auto key_value = dsl::p<flag_key> + dsl::lit_c<'='> + dsl::p<flag_value>;
        // no key, just value
        auto value = dsl::nullopt + dsl::p<flag_value>;

        // We have a key if we're having an equal sign before the newline.
        auto key_condition = dsl::lookahead(dsl::lit_c<'='>, dsl::newline);
        return (key_condition >> key_value | dsl::else_ >> value) + dsl::eol;
    }();

    static constexpr auto value = lexy::construct<flag>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    auto [key, value] = result.value();
    if (key)
        std::printf("The value of '%s' is: %s", key.value().c_str(), value.c_str());
    else
        std::printf("The value: %s", value.c_str());
}

