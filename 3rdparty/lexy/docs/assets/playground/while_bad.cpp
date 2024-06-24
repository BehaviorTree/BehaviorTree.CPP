// INPUT:aaa
struct production
{
    static constexpr auto rule = dsl::while_(dsl::lit_c<'a'>) + dsl::lit_c<'a'>;
};
