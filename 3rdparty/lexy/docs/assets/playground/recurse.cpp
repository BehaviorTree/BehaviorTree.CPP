// INPUT:(((atom)))
struct production
{
    static constexpr auto rule
        // Either we parse ourselves surrounded by expressions, or an atom.
        = dsl::parenthesized(dsl::recurse<production>) | LEXY_LIT("atom");
};
