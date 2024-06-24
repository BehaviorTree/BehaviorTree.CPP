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
        auto item = dsl::integer<int>;
        auto sep  = dsl::trailing_sep(dsl::comma);
        return dsl::times<3>(item, sep);
    }();

    static constexpr auto value
        = lexy::callback<int>([](int a, int b, int c) { return a + b + c; });
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The sum is: %d\n", result.value());
}
