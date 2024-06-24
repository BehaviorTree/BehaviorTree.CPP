// INPUT:<br/>
struct production
{
    static constexpr auto rule = [] {
        auto tag_name = dsl::identifier(dsl::ascii::alpha);

        // If we're having tag name followed by a `/`,
        // it is an empty element without content.
        // Immediately return in that case.
        auto if_empty  = dsl::if_(dsl::lit_c<'/'> >> dsl::return_);
        auto open_tag  = dsl::angle_bracketed(tag_name + if_empty);
        auto close_tag = dsl::angle_bracketed(dsl::lit_c<'/'> + tag_name);

        // Placeholder content.
        auto content = LEXY_LIT("content");

        return open_tag + content + close_tag;
    }();
};
