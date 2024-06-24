// INPUT:3.14
struct production
{
    static constexpr auto rule = [] {
        auto integer  = dsl::digits<>.no_leading_zero();
        auto fraction = dsl::digits<>;

        return integer + dsl::if_(dsl::period >> fraction);
    }();
};
