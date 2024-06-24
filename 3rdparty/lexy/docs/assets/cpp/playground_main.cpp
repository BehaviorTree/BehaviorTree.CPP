#ifndef LEXY_TEST
#    line 1 "playground.cpp"
#endif

namespace
{
struct print_quoted_iterator
{
    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    print_quoted_iterator& operator=(char c)
    {
        if (c == '"')
            std::fputs(R"(\")", stdout);
        else if (c == '\\')
            std::fputs(R"(\\)", stdout);
        else
            std::fputc(c, stdout);
        return *this;
    }
};
} // namespace

int main(int, char* argv[])
{
    auto input = lexy_ext::compiler_explorer_input();

    if (argv[1] == lexy::_detail::string_view("tree"))
    {
        lexy::parse_tree_for<decltype(input)> tree;
        auto                                  result
            = lexy::parse_as_tree<LEXY_PLAYGROUND_PRODUCTION>(tree, input, lexy_ext::report_error);

        std::puts("graph \"Parse Tree\" {");
        std::puts("bgcolor=transparent");
        for (auto [event, node] : tree.traverse())
        {
            switch (event)
            {
            case lexy::traverse_event::enter:
                std::printf(
                    "\"node-%p\" [label=\"%s\", tooltip=\"production\", shape=ellipse, style=bold];\n",
                    node.address(), node.kind().name());
                break;

            case lexy::traverse_event::exit:
                // Now we can add all the connections.
                for (auto child : node.children())
                    std::printf("\"node-%p\" -- \"node-%p\";\n", node.address(), child.address());
                break;

            case lexy::traverse_event::leaf:
                std::printf("\"node-%p\" [label=\"", node.address());
                if (node.lexeme().empty())
                {
                    std::printf("%s\"", node.kind().name());
                }
                else
                {
                    lexy::visualize_to(print_quoted_iterator{}, node.lexeme(),
                                       {lexy::visualize_use_unicode | lexy::visualize_use_symbols
                                        | lexy::visualize_space});
                    std::puts("\", style=filled");
                }

                std::puts("shape=box");
                if (node.kind() == lexy::error_token_kind)
                    std::puts(", fontcolor=red");
                else if (node.kind() == lexy::whitespace_token_kind)
                    std::puts(", fontcolor=gray30");
                std::printf(", tooltip=\"%s\"", node.kind().name());
                std::puts("];");
                break;
            }
        }
        std::puts("}");

        if (result.is_success())
            return 0;
        else if (result.is_recovered_error())
            return 1;
        else if (result.is_fatal_error())
            return 2;
    }
    else if (argv[1] == lexy::_detail::string_view("trace"))
    {
        lexy::trace<LEXY_PLAYGROUND_PRODUCTION>(stdout, input, {lexy::visualize_fancy});
        return 0;
    }
    else
    {
        std::fputs("invalid argument", stderr);
        return 3;
    }
}

