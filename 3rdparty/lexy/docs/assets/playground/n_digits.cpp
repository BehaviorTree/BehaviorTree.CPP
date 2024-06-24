// INPUT:\xABCDEF
struct production
{
    static constexpr auto rule = LEXY_LIT("\\x") + dsl::n_digits<2, dsl::hex>;
};
