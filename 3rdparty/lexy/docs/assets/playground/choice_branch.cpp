// INPUT:ab
struct production
{
    static constexpr auto rule = LEXY_LIT("a") >> LEXY_LIT("bc")  // a, then bc
                                 | LEXY_LIT("a") >> LEXY_LIT("b") // a, then b
                                 | LEXY_LIT("bc") | LEXY_LIT("b");
};
