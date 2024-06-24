// INPUT:atom
struct production
{
    static constexpr auto atext
        = LEXY_CHAR_CLASS("atext",
                          dsl::ascii::alpha / dsl::ascii::digit / LEXY_LIT("!") / LEXY_LIT("#")
                              / LEXY_LIT("$") / LEXY_LIT("%") / LEXY_LIT("&") / LEXY_LIT("'")
                              / LEXY_LIT("*") / LEXY_LIT("+") / LEXY_LIT("-") / LEXY_LIT("/")
                              / LEXY_LIT("=") / LEXY_LIT("?") / LEXY_LIT("^") / LEXY_LIT("_")
                              / LEXY_LIT("`") / LEXY_LIT("{") / LEXY_LIT("|") / LEXY_LIT("}"));

    static constexpr auto rule = dsl::identifier(atext);
};
