// INPUT:reserved__id
struct production
{
    static constexpr auto rule = [] {
        // Define the general identifier syntax.
        auto head = dsl::ascii::alpha_underscore;
        auto tail = dsl::ascii::alpha_digit_underscore;
        auto id   = dsl::identifier(head, tail);

        // Define some keywords.
        auto kw_int    = LEXY_KEYWORD("int", id);
        auto kw_struct = LEXY_KEYWORD("struct", id);
        // ...

        // Parse an identifier
        return id
            // ... that is not a keyword,
            .reserve(kw_int, kw_struct)
            // ... doesn't start with an underscore,
            .reserve_prefix(dsl::lit_c<'_'>)
            // ... or contains a double underscore.
            .reserve_containing(LEXY_LIT("__"));
    }();
};
