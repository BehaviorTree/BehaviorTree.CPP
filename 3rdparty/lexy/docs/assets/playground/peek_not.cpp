// INPUT:Hello World   \n
struct production
{
    struct trailing_spaces
    {
        static constexpr auto name = "trailing spaces";
    };

    static constexpr auto rule
        = LEXY_LIT("Hello World")
          + dsl::peek_not(dsl::ascii::space).error<trailing_spaces> + dsl::eof;
};
