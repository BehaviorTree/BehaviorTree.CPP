// INPUT:--1
struct production : lexy::expression_production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom       = dsl::integer<int>;

    struct operation : dsl::prefix_op
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'-'>);
        using operand            = dsl::atom;
    };
};
