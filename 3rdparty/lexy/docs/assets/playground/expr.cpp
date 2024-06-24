// INPUT:3*2+-1
struct production : lexy::expression_production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom       = dsl::integer<int>;

    struct prefix : dsl::prefix_op
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'-'>);
        using operand            = dsl::atom;
    };

    struct product : dsl::infix_op_left
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'*'>) / dsl::op(dsl::lit_c<'/'>);
        using operand            = prefix;
    };

    struct sum : dsl::infix_op_left
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'+'>) / dsl::op(dsl::lit_c<'-'>);
        using operand            = product;
    };

    using operation = sum;
};
