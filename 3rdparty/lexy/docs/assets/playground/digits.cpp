// INPUT:4'294'967'295
struct production
{
    static constexpr auto rule
        = dsl::digits<>.sep(dsl::digit_sep_tick).no_leading_zero() + dsl::eof;
};
