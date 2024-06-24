// INPUT:abc
struct production
{
    static constexpr auto rule //
        = dsl::literal_set(LEXY_LIT("a"), LEXY_LIT("abc"), LEXY_LIT("bc"));
};
