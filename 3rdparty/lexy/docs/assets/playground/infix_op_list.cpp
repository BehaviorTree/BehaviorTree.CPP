// INPUT:1+2+3
struct production : lexy::expression_production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom       = dsl::integer<int>;

    struct operation : dsl::infix_op_list
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'+'>);
        using operand            = dsl::atom;
    };
};
