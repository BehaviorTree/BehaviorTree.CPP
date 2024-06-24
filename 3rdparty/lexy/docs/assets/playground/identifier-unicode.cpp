// INPUT:ïdéntiför_123
struct production
{
    static constexpr auto rule
        = dsl::identifier(dsl::unicode::xid_start_underscore, // want '_' as well
                          dsl::unicode::xid_continue);
};
