// INPUT:ab
struct production
{
    static constexpr auto rule = LEXY_LIT("a") | LEXY_LIT("ab");
};
