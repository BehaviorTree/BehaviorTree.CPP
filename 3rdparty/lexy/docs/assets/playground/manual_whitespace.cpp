// INPUT:Hello\nWorld   !\n
struct production
{
    static constexpr auto rule = [] {
        auto ws = dsl::whitespace(dsl::ascii::space);
        return LEXY_LIT("Hello") + ws + LEXY_LIT("World") //
               + ws + dsl::exclamation_mark + ws + dsl::eof;
    }();
};
