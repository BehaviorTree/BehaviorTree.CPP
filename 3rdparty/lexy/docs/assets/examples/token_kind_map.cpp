#include <lexy/action/parse_as_tree.hpp>
#include <lexy/dsl.hpp>
#include <lexy/visualize.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

struct name
{
    static constexpr auto rule
        // One or more alpha numeric characters, underscores or hyphens.
        = dsl::identifier(dsl::ascii::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);
};

struct production
{
    // Allow arbitrary spaces between individual tokens.
    // Note that this includes the individual characters of the name.
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto greeting = LEXY_LIT("Hello");
        return greeting + dsl::p<name> + dsl::exclamation_mark + dsl::eof;
    }();
};

//{
enum class my_token_kind
{
    greeting,
    exclamation_mark,
};

template <>
constexpr auto lexy::token_kind_map_for<my_token_kind>
    // Start with the empty map.
    = lexy::token_kind_map
          // Map the greeting token.
          .map<my_token_kind::greeting>(LEXY_LIT("Hello"))
          // Map the exclamation token.
          .map<my_token_kind::exclamation_mark>(dsl::exclamation_mark);
//}

const char* token_kind_name(my_token_kind kind)
{
    switch (kind)
    {
    case my_token_kind::greeting:
        return "greeting";
    case my_token_kind::exclamation_mark:
        return "exclamation_mark";
    }

    return "";
}

int main()
{
    auto input = lexy_ext::compiler_explorer_input();

    // Need to specify the token kind we want here.
    lexy::parse_tree_for<decltype(input), my_token_kind> tree;
    auto result = lexy::parse_as_tree<production>(tree, input, lexy_ext::report_error);

    lexy::visualize(stdout, tree, {lexy::visualize_fancy});

    if (!result)
        return 1;
}

