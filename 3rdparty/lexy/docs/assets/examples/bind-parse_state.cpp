#include <string>
#include <vector>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct entry
{
    std::string name;
    int         a, b;
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto integer = dsl::integer<int>;
        return dsl::twice(integer, dsl::sep(dsl::comma));
    }();

    // Construct the entry where the name is taken from the parse state.
    static constexpr auto value
        = lexy::bind(lexy::construct<entry>, lexy::parse_state, lexy::values);
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, "foo", lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("%s: %d, %d\n", result.value().name.c_str(), result.value().a, result.value().b);
}

