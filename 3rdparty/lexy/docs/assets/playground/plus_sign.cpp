// INPUT:+123
struct production
{
    static constexpr auto rule
        // Plus sign followed by a decimal integer.
        = dsl::plus_sign + dsl::integer<int>;
};
