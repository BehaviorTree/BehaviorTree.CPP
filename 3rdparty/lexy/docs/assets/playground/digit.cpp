// INPUT:A
struct production
{
    static constexpr auto rule = dsl::digit<dsl::hex_upper> + dsl::eof;
};
