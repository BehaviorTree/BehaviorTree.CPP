// INPUT:Hello\nWorld   !
struct production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule                                      //
        = dsl::no_whitespace(LEXY_LIT("Hello") + LEXY_LIT("World")) //
          + dsl::exclamation_mark + dsl::eof;
};
