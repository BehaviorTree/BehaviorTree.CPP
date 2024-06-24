// INPUT:Hello World!
struct name
{
    static constexpr auto rule
        // One or more alpha numeric characters, underscores or hyphens.
        = dsl::identifier(dsl::unicode::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);
};

struct production
{
    // Allow arbitrary spaces between individual tokens.
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto greeting = LEXY_LIT("Hello");
        return greeting + dsl::p<name> + dsl::exclamation_mark + dsl::eof;
    }();
};
