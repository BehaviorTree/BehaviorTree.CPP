// INPUT:integer
struct production
{
    static constexpr auto rule = [] {
        // Define the general identifier syntax.
        auto head = dsl::ascii::alpha_underscore;
        auto tail = dsl::ascii::alpha_digit_underscore;
        auto id   = dsl::identifier(head, tail);

        // Parse a keyword.
        return LEXY_KEYWORD("int", id);
    }();
};
