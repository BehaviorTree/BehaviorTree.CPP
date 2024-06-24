#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>
#include <string>

namespace dsl = lexy::dsl;

//{
struct production
{
    // A mapping of the simple escape sequences to their replacement values.
    static constexpr auto escaped_symbols = lexy::symbol_table<char> //
                                                .map<'"'>('"')
                                                .map<'\\'>('\\')
                                                .map<'/'>('/')
                                                .map<'b'>('\b')
                                                .map<'f'>('\f')
                                                .map<'n'>('\n')
                                                .map<'r'>('\r')
                                                .map<'t'>('\t');

    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = -dsl::ascii::control;

        // Escape sequences start with a backlash.
        // They either map one of the symbols,
        // or a Unicode code point of the form uXXXX.
        auto escape = dsl::backslash_escape //
                          .symbol<escaped_symbols>()
                          .rule(dsl::lit_c<'u'> >> dsl::code_point_id<4>);
        return dsl::quoted(c, escape);
    }();

    // Need to specify a target encoding to handle the code point.
    static constexpr auto value = lexy::as_string<std::string, lexy::utf8_encoding>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The string is: %s\n", result.value().c_str());
}

