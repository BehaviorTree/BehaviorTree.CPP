// INPUT:"   "
struct production
{
    static constexpr auto rule //
        = dsl::quoted(dsl::ascii::space & dsl::ascii::print);
};
