// INPUT:1++++
struct production : lexy::expression_production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom       = dsl::integer<int>;

    struct operation : dsl::postfix_op
    {
        static constexpr auto op = dsl::op(LEXY_LIT("++"));
        using operand            = dsl::atom;
    };
};
