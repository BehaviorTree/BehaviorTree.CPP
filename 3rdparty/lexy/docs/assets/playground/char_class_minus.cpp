// INPUT:ABX
struct production
{
    static constexpr auto rule //
        = dsl::identifier(dsl::ascii::upper - dsl::lit_c<'X'>);
};
