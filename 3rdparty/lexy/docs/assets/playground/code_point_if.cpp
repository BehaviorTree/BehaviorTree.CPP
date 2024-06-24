// INPUT:a
struct production
{
    struct even
    {
        constexpr bool operator()(lexy::code_point cp)
        {
            return cp.value() % 2 == 0;
        }
    };

    static constexpr auto rule = dsl::code_point.if_<even>() + dsl::eof;
};
