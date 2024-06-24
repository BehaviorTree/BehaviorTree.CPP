#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
enum class cv_qualifier
{
    none      = 0,
    const_    = 1 << 1,
    volatile_ = 1 << 2,
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    // Map cv-qualifiers to their value.
    static constexpr auto cv = lexy::symbol_table<cv_qualifier> //
                                   .map<LEXY_SYMBOL("const")>(cv_qualifier::const_)
                                   .map<LEXY_SYMBOL("volatile")>(cv_qualifier::volatile_);

    // Parse any combination of cv qualifiers.
    static constexpr auto rule = dsl::flags(dsl::symbol<cv>(dsl::identifier(dsl::ascii::alpha)));

    static constexpr auto value = lexy::forward<cv_qualifier>;
};
//}

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    if ((int(result.value()) & int(cv_qualifier::const_)) != 0)
        std::puts("const");
    if ((int(result.value()) & int(cv_qualifier::volatile_)) != 0)
        std::puts("volatile");
}

