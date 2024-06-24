// INPUT:{\n  foo();\n  error1;\n  bar();\n  error2\n  foo();\n  error3\n}
struct statement
{
    static constexpr auto rule = [] {
        // A statement is terminated by a semicolon.
        // Error recovery fails when we've reached the } of the scope.
        auto terminator = dsl::terminator(dsl::semicolon).limit(dsl::lit_c<'}'>);
        return terminator.opt(LEXY_LIT("foo()") | LEXY_LIT("bar()"));
    }();
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    // Just a list of statements surrounded by {}.
    static constexpr auto rule = dsl::curly_bracketed.list(dsl::p<statement>);
};

