#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto rule = [] {
        auto digits = dsl::digits<>.sep(dsl::digit_sep_tick).no_leading_zero();
        return dsl::integer<int>(digits);
    }();

    static constexpr auto value = lexy::as_integer<int>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result.has_value())
        return 1;

    std::printf("The value is: %d\n", result.value());
    return result ? 0 : 1;
}

