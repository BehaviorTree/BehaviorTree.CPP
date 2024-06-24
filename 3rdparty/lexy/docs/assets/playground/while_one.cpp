// INPUT:Hello
struct production
{
    static constexpr auto rule = dsl::while_one(dsl::ascii::alpha);
};
