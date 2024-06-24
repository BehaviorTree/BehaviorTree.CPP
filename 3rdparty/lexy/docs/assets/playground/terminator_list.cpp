// INPUT:abc,12,abc123,123.
struct production
{
    static constexpr auto rule = [] {
        auto item = LEXY_LIT("abc") | LEXY_LIT("123");

        auto terminator = dsl::terminator(dsl::period);
        return terminator.list(item, dsl::sep(dsl::comma));
    }();
};
