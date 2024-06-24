// INPUT:function foo(...)\n{\n  ...\n}
struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto id          = dsl::identifier(dsl::ascii::alpha);
        auto kw_function = LEXY_KEYWORD("function", id);

        auto arguments = dsl::parenthesized(LEXY_LIT("..."));
        auto body      = dsl::curly_bracketed(LEXY_LIT("..."));

        // The position of a function is the first character of the name.
        return kw_function + dsl::position + id + arguments + body;
    }();
};
