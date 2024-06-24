// INPUT:Hello World
struct production
{
    static constexpr auto rule = [] {
        auto word = dsl::while_one(dsl::ascii::alpha);
        return dsl::do_while(word, dsl::ascii::space);
    }();
};
