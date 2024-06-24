// INPUT:identifier_123
struct production
{
    static constexpr auto rule = [] {
        auto head = dsl::ascii::alpha_underscore;
        auto tail = dsl::ascii::alpha_digit_underscore;
        return dsl::identifier(head, tail);
    }();
};
