#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production : lexy::expression_production
{
    static constexpr auto atom = dsl::integer<int>;

    struct operation : dsl::infix_op_left
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'+'>);
        using operand            = dsl::atom;
    };

    static constexpr auto value
        = lexy::callback<int>([](int value) { return value; },
                              [](int lhs, lexy::op<operation::op>, int rhs) { return lhs + rhs; });
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("Result: %d\n", result.value());
}

