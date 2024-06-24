#include <cstdio>
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>

namespace dsl = lexy::dsl;

struct production
{
    static constexpr auto rule  = dsl::integer<int>;
    static constexpr auto value = lexy::forward<int>;
};

//{
int main()
{
    auto input = lexy_ext::compiler_explorer_input();

    // Parse the production, but ignore all errors.
    auto result = lexy::parse<production>(input, lexy::noop);
    if (!result)
        // Note that parsing can still fail.
        return 1;

    std::printf("The value is: %d\n", result.value());
}
//}

