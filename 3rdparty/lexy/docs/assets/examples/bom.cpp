#include <lexy/action/validate.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto rule = [] {
        auto bom = dsl::bom<lexy::utf8_encoding,
                            // Doesn't matter for UTF-8.
                            lexy::encoding_endianness::little>;
        return dsl::opt(bom) + LEXY_LIT("Hello!") + dsl::eof;
    }();
};
//}

int main()
{
    const unsigned char data[] = {0xEF, 0xBB, 0xBF, 'H', 'e', 'l', 'l', 'o', '!', '\0'};
    auto                input  = lexy::zstring_input(data);
    auto                result = lexy::validate<production>(input, lexy_ext::report_error);
    return result ? 0 : 1;
}

