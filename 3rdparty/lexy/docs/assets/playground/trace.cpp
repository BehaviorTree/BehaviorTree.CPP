// INPUT:Hello abx!
struct name
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha);
};

struct alphabet
{
    static constexpr auto rule
        // Just something stupid, so we can see a backtrack.
        = dsl::peek(LEXY_LIT("abc")) >> LEXY_LIT("abcdefg");
};

struct number
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::digit);
};

struct object
{
    struct unexpected
    {
        static constexpr auto name = "unexpected";
    };

    static constexpr auto rule
        = dsl::p<alphabet> | dsl::p<name> | dsl::p<number>
         // Issue an error, but recover.
         | dsl::try_(dsl::error<unexpected>);
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto greeting = LEXY_LIT("Hello");
        return greeting + LEXY_DEBUG("finished greeting") //
               + dsl::p<object> + dsl::exclamation_mark;
    }();
};
