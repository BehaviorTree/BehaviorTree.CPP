// INPUT:;
struct production
{
    static constexpr auto rule = [] {
        auto terminator = dsl::terminator(dsl::semicolon);
        return terminator.opt(LEXY_LIT("statement"));
    }();
};
