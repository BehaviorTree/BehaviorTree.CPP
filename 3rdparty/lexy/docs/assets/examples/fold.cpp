#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        // An item is a point (x, y)
        auto integer = dsl::integer<int>;
        auto item    = dsl::parenthesized(dsl::twice(integer, dsl::sep(dsl::comma)));

        return dsl::list(item, dsl::sep(dsl::comma));
    }();

    // Sum the x components of the points.
    static constexpr auto value
        = lexy::fold<int>(0, [](int current, int x, int) { return current + x; });
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The value is: %d\n", result.value());
}

