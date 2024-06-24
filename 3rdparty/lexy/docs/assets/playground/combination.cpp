// INPUT:cab
struct production
{
    static constexpr auto rule
        = dsl::combination(dsl::lit_c<'a'>, dsl::lit_c<'b'>, dsl::lit_c<'c'>);
};
