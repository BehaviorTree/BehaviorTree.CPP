// INPUT:aaabb
struct production
{
    struct mismatch
    {};

    static constexpr auto rule = [] {
        // Declare a counter - it is not created yet!
        auto counter = dsl::context_counter<production>;

        // Parse a sequence of 'a' and add the number to it.
        auto a = counter.push(dsl::while_(dsl::lit_c<'a'>));
        // Parse a sequence of 'b' and subtract the number from it.
        auto b = counter.pop(dsl::while_(dsl::lit_c<'b'>));

        // Create the counter initialized to zero,
        // parse the two, and require its back to zero.
        return counter.create() + a + b + dsl::must(counter.is_zero()).error<mismatch>;
    }();
};
