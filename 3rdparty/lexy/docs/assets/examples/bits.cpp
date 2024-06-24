#include <lexy/action/validate.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct code_point
{
    static constexpr auto rule = [] {
        // 10xxxxxx
        auto continuation = dsl::bits(dsl::bit::_1, dsl::bit::_0, dsl::bit::any<6>);

        // 0xxxxxxx
        auto ascii = dsl::bits(dsl::bit::_0, dsl::bit::any<7>);
        // 110xxxxx
        auto lead_two = dsl::bits(dsl::bit::_1, dsl::bit::_1, dsl::bit::_0, dsl::bit::any<5>);
        // 1110xxxx
        auto lead_three
            = dsl::bits(dsl::bit::_1, dsl::bit::_1, dsl::bit::_1, dsl::bit::_0, dsl::bit::any<4>);
        // 11110xxx
        auto lead_four = dsl::bits(dsl::bit::_1, dsl::bit::_1, dsl::bit::_1, dsl::bit::_1,
                                   dsl::bit::_0, dsl::bit::any<3>);

        return ascii | lead_two >> continuation | lead_three >> dsl::twice(continuation)
               | lead_four >> dsl::times<3>(continuation);
    }();
};
//}

struct production
{
    static constexpr auto rule = dsl::p<code_point> + dsl::eof;
};

int main()
{
    unsigned char bytes[] = {0xE2, 0x82, 0xAC};
    auto          input   = lexy::string_input(bytes, sizeof(bytes));

    auto result = lexy::validate<production>(input, lexy_ext::report_error);
    return result ? 0 : 1;
}

