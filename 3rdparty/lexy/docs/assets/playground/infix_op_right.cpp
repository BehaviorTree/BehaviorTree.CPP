// INPUT:1**2**3
struct production : lexy::expression_production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom       = dsl::integer<int>;

    struct operation : dsl::infix_op_right
    {
        static constexpr auto op = dsl::op(LEXY_LIT("**"));
        using operand            = dsl::atom;
    };
};
