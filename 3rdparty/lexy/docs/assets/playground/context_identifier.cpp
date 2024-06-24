// INPUT:Hello,Hallo
struct production
{
    static constexpr auto rule = [] {
        // Declare an identifier variable - it is not created yet!
        auto word     = dsl::identifier(dsl::ascii::alpha);
        auto word_var = dsl::context_identifier<production>(word);

        // Parse a word and capture it in the variable.
        auto first_word = word_var.capture();
        // Parse another word and compare it agains the variable.
        auto second_word = word_var.rematch();

        // Create the empty variable, then parse the two words.
        return word_var.create() + first_word + dsl::lit_c<','> + second_word;
    }();
};
