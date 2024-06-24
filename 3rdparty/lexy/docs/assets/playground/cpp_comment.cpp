// INPUT:// This is a comment.
struct production
{
    static constexpr auto rule = LEXY_LIT("//") + dsl::until(dsl::newline).or_eof();
};
