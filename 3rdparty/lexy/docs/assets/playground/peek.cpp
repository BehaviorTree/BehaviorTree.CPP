// INPUT:-11
struct production
{
    static constexpr auto rule = [] {
        // Number with optional minus sign.
        auto number = dsl::minus_sign + dsl::digits<>;

        // Only parse a number if we have a minus or digit.
        auto condition = dsl::peek(dsl::lit_c<'-'> / dsl::digit<>);
        return dsl::if_(condition >> number);
    }();
};
