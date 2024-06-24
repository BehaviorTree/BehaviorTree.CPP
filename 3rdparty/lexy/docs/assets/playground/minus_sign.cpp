// INPUT:-123
struct production
{
    static constexpr auto rule
        // Minus sign followed by a decimal integer.
        = dsl::minus_sign + dsl::integer<int>;
};
