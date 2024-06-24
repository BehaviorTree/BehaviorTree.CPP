#include <lexy/action/scan.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct control_production
{
    // Allow ASCII whitespace.
    static constexpr auto whitespace = dsl::ascii::space;
};

int main()
{
    // Construct a scanner for the input.
    auto input   = lexy_ext::compiler_explorer_input();
    auto scanner = lexy::scan<control_production>(input, lexy_ext::report_error);

    // Parse two integers separated by comma.
    auto x = scanner.integer<int>(dsl::digits<>);
    scanner.parse(dsl::comma);
    auto y = scanner.integer<int>(dsl::digits<>);

    std::printf("%d, %d", x.value_or(-1), y.value_or(-1));
}
//}

