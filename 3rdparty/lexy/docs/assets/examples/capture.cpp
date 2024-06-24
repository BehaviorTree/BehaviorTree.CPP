#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
// The type of a lexy::lexeme depends on the input.
using lexeme = lexy_ext::compiler_explorer_lexeme;

struct production
{
    static constexpr auto rule = dsl::capture(dsl::code_point);

    // Same as `lexy::as_string<std::string>`.
    static constexpr auto value = lexy::callback<std::string>(
        [](lexeme lex) { return std::string(lex.begin(), lex.end()); });
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("Codepoint: %s (%zu code units)\n", result.value().c_str(), result.value().size());
}

