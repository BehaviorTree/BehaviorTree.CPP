// INPUT:==
struct production
{
    static constexpr auto rule
        // = but then not another =
        = dsl::not_followed_by(dsl::lit_c<'='>, dsl::lit_c<'='>);
};
