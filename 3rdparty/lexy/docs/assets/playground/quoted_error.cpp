// INPUT:"Hello\nWorld"
struct production
{
    struct invalid_character
    {
        static constexpr auto name = "invalid character";
    };

    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = (-dsl::ascii::control).error<invalid_character>;

        return dsl::quoted(c);
    }();
};
