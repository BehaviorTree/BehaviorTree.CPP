// INPUT:Hello
struct production
{
    static constexpr auto rule = dsl::while_(dsl::ascii::alpha);
};
