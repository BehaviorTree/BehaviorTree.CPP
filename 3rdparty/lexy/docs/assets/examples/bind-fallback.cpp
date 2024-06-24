#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct decimal
{
    int         integer;
    std::string fraction;
};

struct production
{
    struct fraction
    {
        static constexpr auto rule  = dsl::capture(dsl::digits<>);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    static constexpr auto rule = [] {
        auto integer = dsl::integer<int>;

        return integer + dsl::opt(dsl::period >> dsl::p<fraction>);
    }();

    static constexpr auto value = lexy::bind(lexy::construct<decimal>,
                                             // If the second argument is lexy::nullopt,
                                             // produce a zero instead.
                                             lexy::_1, lexy::_2 or "0");
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The value is: %d.%s\n", result.value().integer, result.value().fraction.c_str());
}

