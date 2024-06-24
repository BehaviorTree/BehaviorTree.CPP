// INPUT:Hallo
struct production
{
    static constexpr auto rule
        // Input should be empty if greeting isn't known.
        = LEXY_LIT("Hello") | LEXY_LIT("Hi") | dsl::else_ >> dsl::eof;
};
