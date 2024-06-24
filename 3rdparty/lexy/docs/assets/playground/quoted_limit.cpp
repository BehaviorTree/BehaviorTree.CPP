// INPUT:"Hello World!\n
struct production
{
    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = -dsl::ascii::control;

        // If we have a newline inside our string, we're missing the closing ".
        auto quoted = dsl::quoted.limit(dsl::ascii::newline);
        return quoted(c);
    }();
};
