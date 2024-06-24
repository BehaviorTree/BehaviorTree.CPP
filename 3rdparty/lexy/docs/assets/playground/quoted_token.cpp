// INPUT:  "  Hello  World!  "  ;
struct quoted : lexy::token_production
{
    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = -dsl::ascii::control;

        return dsl::quoted(c);
    }();
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule       = dsl::p<quoted> + dsl::semicolon;
};

