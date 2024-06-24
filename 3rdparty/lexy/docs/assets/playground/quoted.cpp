// INPUT:"Hello World!"
struct production
{
    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = -dsl::ascii::control;

        return dsl::quoted(c);
    }();
};
