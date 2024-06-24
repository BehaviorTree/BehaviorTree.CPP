// INPUT:+(+inner+normal+)+,+(-inner-override-)-,+(innertoken)+,+(_inner_token_whitespace)_+.+
// An inner production that does not override the whitespace.
struct inner_normal
{
    // After every token in this rule, the whitespace is '+',
    // as determined by its root production `production`.
    static constexpr auto rule //
        = dsl::parenthesized(LEXY_LIT("inner") + LEXY_LIT("normal"));
};

// An inner production that overrides the current whitespace definition.
struct inner_override
{
    static constexpr auto whitespace = dsl::lit_c<'-'>;

    // After every token in this rule, the whitespace is '-',
    // as determined by the `whitespace` member of the current production.
    static constexpr auto rule //
        = dsl::parenthesized(LEXY_LIT("inner") + LEXY_LIT("override"));
};

// A token production that does not have inner whitespace.
struct inner_token : lexy::token_production
{
    struct inner_inner
    {
        // No whitespace is skipped here, as its root production is `inner_token`,
        // which does not have a `whitespace` member.
        static constexpr auto rule = LEXY_LIT("inner") + LEXY_LIT("token");
    };

    // No whitespace is skipped here, as the current production inherits from
    // `lexy::token_production`.
    static constexpr auto rule = dsl::parenthesized(dsl::p<inner_inner>);
};

// A token production that does have inner whitespace, but different one.
struct inner_token_whitespace : lexy::token_production
{
    struct inner_inner
    {
        // After every token in this rule, the whitespace is '_',
        // as determined by its root production `inner_token_whitespace`.
        static constexpr auto rule //
            = LEXY_LIT("inner") + LEXY_LIT("token") + LEXY_LIT("whitespace");
    };

    static constexpr auto whitespace = dsl::lit_c<'_'>;

    static constexpr auto rule = dsl::parenthesized(dsl::p<inner_inner>);
};

// The root production defines whitespace.
struct production
{
    static constexpr auto whitespace = dsl::lit_c<'+'>;

    // After every token in this rule, the whitespace is '+',
    // as determined by the `whitespace` member of the current production.
    // Whitespace is also skipped after the two token productions.
    static constexpr auto rule
        = dsl::p<inner_normal> + dsl::comma + dsl::p<inner_override> + dsl::comma
          + dsl::p<inner_token> + dsl::comma + dsl::p<inner_token_whitespace> //
          + dsl::period + dsl::eof;
};
