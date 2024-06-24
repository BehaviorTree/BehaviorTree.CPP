// INPUT:rgb(255,0,255)
struct channel_hex
{
    static constexpr auto rule = dsl::integer<std::uint8_t>(dsl::n_digits<2, dsl::hex>);
};

struct channel_dec
{
    static constexpr auto rule = dsl::integer<std::uint8_t>;
};

struct color
{
    static constexpr auto rule = [] {
        auto hex_color = dsl::hash_sign >> dsl::times<3>(dsl::p<channel_hex>);

        auto dec_channels = dsl::times<3>(dsl::p<channel_dec>, dsl::sep(dsl::comma));
        auto fnc_color    = LEXY_LIT("rgb") >> dsl::parenthesized(dec_channels);

        return hex_color | fnc_color;
    }();
};

using production = color;
