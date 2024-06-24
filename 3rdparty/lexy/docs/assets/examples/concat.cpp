#include <vector>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct list
{
    static constexpr auto rule = [] {
        auto integer = dsl::integer<int>;
        return dsl::list(integer, dsl::sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<std::vector<int>>;
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule       = dsl::list(dsl::p<list>, dsl::sep(dsl::newline));
    static constexpr auto value      = lexy::concat<std::vector<int>>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("numbers: ");
    for (auto i : result.value())
        std::printf("%d ", i);
    std::putchar('\n');
}

