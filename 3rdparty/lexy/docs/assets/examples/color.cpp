#include <cstdio>
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

struct Color
{
    std::uint8_t r, g, b;
};

namespace grammar
{
namespace dsl = lexy::dsl;

struct channel
{
    static constexpr auto rule  = dsl::integer<std::uint8_t>(dsl::n_digits<2, dsl::hex>);
    static constexpr auto value = lexy::forward<std::uint8_t>;
};

struct color
{
    static constexpr auto rule  = dsl::hash_sign + dsl::times<3>(dsl::p<channel>);
    static constexpr auto value = lexy::construct<Color>;
};
} // namespace grammar

int main()
{
    auto input = lexy_ext::compiler_explorer_input(); // special input for CompilerExplorer examples
    auto result = lexy::parse<grammar::color>(input, lexy_ext::report_error);
    if (result.has_value())
    {
        auto color = result.value();
        std::printf("#%02x%02x%02x\n", color.r, color.g, color.b);
    }

    return result ? 0 : 1;
}

