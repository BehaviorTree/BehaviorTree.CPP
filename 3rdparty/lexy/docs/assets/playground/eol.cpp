// INPUT:Hello
struct production
{
    static constexpr auto rule = LEXY_LIT("Hello") + dsl::eol;
};

