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
    std::string      name;
    std::vector<int> numbers;
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto integer = dsl::integer<int>;
        return dsl::square_bracketed.list(integer, dsl::sep(dsl::comma));
    }();

    static constexpr auto value
        // Collect all the numbers in a vector, then turn the result into an entry.
        = lexy::as_list<std::vector<int>> >> lexy::callback<entry>([](std::vector<int>&& vec) {
              return entry{"foo", std::move(vec)};
          });
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("%s: ", result.value().name.c_str());
    for (auto i : result.value().numbers)
        std::printf("%d ", i);
    std::putchar('\n');
}

