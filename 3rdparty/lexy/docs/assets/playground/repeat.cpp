// INPUT:3aaa
struct production
{
    static constexpr auto rule
        // The number of 'a's is determined by the integer value.
        = dsl::repeat(dsl::integer<int>)(dsl::lit_c<'a'>);
};
