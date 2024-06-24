// INPUT:Hello /* C comment */\nWorld   !
struct production
{
    // Note that an unterminated C comment will raise an error.
    static constexpr auto whitespace
        = dsl::ascii::space | LEXY_LIT("/*") >> dsl::until(LEXY_LIT("*/"));

    static constexpr auto rule //
        = LEXY_LIT("Hello") + LEXY_LIT("World") + dsl::exclamation_mark;
};
