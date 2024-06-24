// INPUT:#FF00FF
struct channel
{
    static constexpr auto rule = dsl::n_digits<2, dsl::hex>;
};

struct color
{
    static constexpr auto rule = dsl::hash_sign + dsl::times<3>(dsl::p<channel>) + dsl::eof;
};

using production = color;
