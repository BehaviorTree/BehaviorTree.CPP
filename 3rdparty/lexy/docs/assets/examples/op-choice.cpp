#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
constexpr auto op_plus  = dsl::op(dsl::lit_c<'+'>);
constexpr auto op_minus = dsl::op(dsl::lit_c<'-'>);

struct production : lexy::expression_production
{
    static constexpr auto atom = dsl::integer<int>;

    struct operation : dsl::infix_op_left
    {
        static constexpr auto op = op_plus / op_minus;
        using operand            = dsl::atom;
    };

    static constexpr auto value
        = lexy::callback<int>([](int value) { return value; },
                              [](int lhs, lexy::op<op_plus>, int rhs) { return lhs + rhs; },
                              [](int lhs, lexy::op<op_minus>, int rhs) { return lhs - rhs; });
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

