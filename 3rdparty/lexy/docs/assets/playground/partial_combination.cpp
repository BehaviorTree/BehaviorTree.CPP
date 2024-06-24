// INPUT:ca
struct production
{
    static constexpr auto rule
        = dsl::partial_combination(dsl::lit_c<'a'>, dsl::lit_c<'b'>, dsl::lit_c<'c'>);
};
