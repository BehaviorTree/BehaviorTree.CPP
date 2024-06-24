// INPUT:((((atom))))
struct production
{
    // We define a (tiny) maximum recursion depth.
    // This prevents unbounded recursion which can cause a stack overflow.
    static constexpr auto max_recursion_depth = 3;

    static constexpr auto rule
        // Either we parse ourselves surrounded by expressions, or an atom.
        = dsl::parenthesized(dsl::recurse<production>) | LEXY_LIT("atom");
};
