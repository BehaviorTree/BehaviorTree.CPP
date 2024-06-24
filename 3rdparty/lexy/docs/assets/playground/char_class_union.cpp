// INPUT:H123
struct production
{
    static constexpr auto rule //
        = dsl::identifier(dsl::ascii::upper / dsl::ascii::digit);
};
