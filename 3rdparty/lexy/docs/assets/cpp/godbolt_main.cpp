//=== main function ===//
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/visualize.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>

int main()
{
    auto input = lexy_ext::compiler_explorer_input();

    lexy::parse_tree_for<decltype(input)> tree;
    auto                                  result
        = lexy::parse_as_tree<LEXY_PLAYGROUND_PRODUCTION>(tree, input, lexy_ext::report_error);

    lexy::visualize(stdout, tree, {lexy::visualize_fancy});

    if (!result)
        return 1;
}

