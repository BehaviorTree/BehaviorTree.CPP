// INPUT:type Id = <error>;\nfunction foo(...) {\n  ...\n}\n
constexpr auto id          = dsl::identifier(dsl::ascii::alpha);
constexpr auto kw_function = LEXY_KEYWORD("function", id);
constexpr auto kw_type     = LEXY_KEYWORD("type", id);

struct function_decl
{
    static constexpr auto rule = [] {
        auto arguments = dsl::parenthesized(LEXY_LIT("..."));
        auto body      = dsl::curly_bracketed(LEXY_LIT("..."));

        return kw_function >> id + arguments + body;
    }();
};

struct type_decl
{
    static constexpr auto rule //
        = kw_type >> id + dsl::lit_c<'='> + id + dsl::semicolon;
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule       = [] {
        auto decl = dsl::p<function_decl> | dsl::p<type_decl>;

        // We recover from any errors by skipping until the next decl.
        auto decl_recover = dsl::find(kw_function, kw_type);
        auto try_decl     = dsl::try_(decl, decl_recover);

        return dsl::list(try_decl);
    }();
};
