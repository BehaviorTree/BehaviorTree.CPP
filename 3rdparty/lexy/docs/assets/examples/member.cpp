#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct point
{
    int x, y;
};

struct production
{
    static constexpr auto rule = [] {
        auto value = dsl::integer<int>;

        // Parse an integer into the x/y member of point.
        auto x_coord = (dsl::member<& point::x> = value);
        auto y_coord = (dsl::member<& point::y> = value);

        return x_coord + dsl::comma + y_coord;
    }();

    // `lexy::as_aggregate` accepts the `lexy::member<Fn>` + value pairs.
    static constexpr auto value = lexy::as_aggregate<point>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The value is: (%d, %d)\n", result.value().x, result.value().y);
}

