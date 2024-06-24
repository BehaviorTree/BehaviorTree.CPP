// INPUT:cccbab
struct production
{
    struct expected_a_before_b
    {
        static constexpr auto name = "expected a before b";
    };

    static constexpr auto rule = [] {
        // Declare a flag - it is not created yet!
        auto flag = dsl::context_flag<production>;

        // Parsing a sets the flag to true.
        auto a = dsl::lit_c<'a'> >> flag.set();
        // Parsing b requires that the flag has been set already.
        auto b = dsl::lit_c<'b'> >> dsl::must(flag.is_reset()).error<expected_a_before_b>;
        // Parsing c doesn't care about the flag.
        auto c = dsl::lit_c<'c'>;

        // Create the flag initialized to false.
        // Then parse a|b|c in a loop.
        return flag.create() + dsl::loop(a | b | c | dsl::break_);
    }();
};
