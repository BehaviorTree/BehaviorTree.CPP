#include <cstdio>

#define LEXY_EXPERIMENTAL 1
#include <lexy/action/parse.hpp>
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/parse_tree_input.hpp>
#include <lexy/parse_tree.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
// A simple grammar for a single key-value pair.
namespace grammar
{
struct key
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alnum);
};

struct integer
{
    static constexpr auto rule = dsl::digits<>;
};

struct key_value_pair
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule       = dsl::p<key> + dsl::lit_c<'='> + dsl::p<integer>;
};
} // namespace grammar

namespace tree_grammar
{
struct integer
{
    // Match the digits token and extract their integer value.
    static constexpr auto rule  = dsl::tnode<lexy::digits_token_kind>(dsl::integer<int>);
    static constexpr auto value = lexy::as_integer<int>;
};

struct key_value_pair
{
    // Skip over literal and whitespace tokens automatically.
    static constexpr auto whitespace
        = dsl::tnode<lexy::literal_token_kind> | dsl::tnode<lexy::whitespace_token_kind>;

    // Skip the key production but parse the children of the integer production.
    static constexpr auto rule = [] {
        auto key   = dsl::pnode<grammar::key>;
        auto value = dsl::pnode<grammar::integer>(dsl::p<integer>);
        return key + value;
    }();

    static constexpr auto value = lexy::forward<int>;
};
} // namespace tree_grammar

int main()
{
    auto input = lexy_ext::compiler_explorer_input();

    // Parse the string into a tree.
    lexy::parse_tree_for<lexy::buffer<lexy::utf8_encoding>> tree;
    if (!lexy::parse_as_tree<grammar::key_value_pair>(tree, input, lexy_ext::report_error))
        return 1;

    // Parse the tree to extract the value (we ignore error reporting here since it's a bug for the
    // parse tree grammar to fail).
    auto result
        = lexy::parse<tree_grammar::key_value_pair>(lexy::parse_tree_input(tree), lexy::noop);
    if (!result)
        return 2;

    std::printf("Value: %d\n", result.value());
}
//}

