// INPUT:echo World
struct production
{
    struct expected_sep
    {
        static constexpr auto name = "expected separator";
    };

    static constexpr auto rule = [] {
        // A separator is either blank or \ + newline.
        // If a separator is not present, raise the specific error.
        auto blank      = dsl::ascii::blank;
        auto escaped_nl = dsl::backslash >> dsl::newline;
        auto sep        = dsl::must(blank | escaped_nl).error<expected_sep>;

        return LEXY_LIT("echo") + sep
               + dsl::identifier(dsl::ascii::alnum)
               // Allow an optional separator before EOL.
               + dsl::if_(sep) + dsl::eol;
    }();
};
