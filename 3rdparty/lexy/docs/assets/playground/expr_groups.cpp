// INPUT:1+2|3
struct production : lexy::expression_production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom       = dsl::integer<int>;

    struct math : dsl::infix_op_left
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'+'>) / dsl::op(dsl::lit_c<'-'>);
        using operand            = dsl::atom;
    };

    struct bits : dsl::infix_op_left
    {
        static constexpr auto op = dsl::op(dsl::lit_c<'|'>) / dsl::op(dsl::lit_c<'&'>);
        using operand            = dsl::atom;
    };

    // Either a math or a bit operator, but not mixing.
    using operation = dsl::groups<math, bits>;
};
