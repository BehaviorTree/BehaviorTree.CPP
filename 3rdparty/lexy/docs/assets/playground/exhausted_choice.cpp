// INPUT:Hey
struct production
{
    static constexpr auto rule = LEXY_LIT("Hello") | LEXY_LIT("Hi");
};
