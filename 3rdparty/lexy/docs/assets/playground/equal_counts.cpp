// INPUT:aaabbcccc
struct production
{
    static constexpr auto rule = [] {
        // Parse 'a's and count them.
        auto ac = dsl::context_counter<struct ac_id>;
        auto a  = ac.create() + ac.push(dsl::while_(dsl::lit_c<'a'>));

        // Parse 'b's and count them.
        auto bc = dsl::context_counter<struct bc_id>;
        auto b  = bc.create() + bc.push(dsl::while_(dsl::lit_c<'b'>));

        // Parse 'c's and count them.
        auto cc = dsl::context_counter<struct cc_id>;
        auto c  = cc.create() + cc.push(dsl::while_(dsl::lit_c<'c'>));

        // Check that they're equal.
        auto check = dsl::equal_counts(ac, bc, cc);

        return a + b + c + check;
    }();
};
