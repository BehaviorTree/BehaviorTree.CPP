#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha);

    static constexpr auto value
        = lexy::as_string<std::string, lexy::ascii_encoding>.case_folding(dsl::ascii::case_folding);
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

