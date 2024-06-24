// INPUT:statement;
struct production
{
    static constexpr auto rule = [] {
        auto terminator = dsl::terminator(dsl::semicolon);
        return terminator(LEXY_LIT("statement"));
    }();
};
