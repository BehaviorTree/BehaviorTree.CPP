// INPUT:r##"Hello "# World!"##
struct production
// We produce a UTF-8 lexeme (i.e. a string view) of the contents.
: lexy::scan_production<lexy::buffer_lexeme<lexy::utf8_encoding>>,
  lexy::token_production
{
    struct unterminated
    {
        static constexpr auto name = "unterminated raw string literal";
    };

    template <typename Context, typename Reader>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        auto open_hash_count = 0;

        // Parse the opening delimiter.
        scanner.parse(dsl::lit_c<'r'>);
        while (scanner.branch(dsl::lit_c<'#'>))
            ++open_hash_count;
        scanner.parse(dsl::lit_c<'"'>);
        if (!scanner)
            return lexy::scan_failed;

        // Parse the contents of the string.
        auto content_begin = scanner.position();
        for (auto closing_hash_count = -1; closing_hash_count != open_hash_count;)
        {
            // Handle the next character.
            if (scanner.branch(dsl::lit_c<'"'>))
            {
                // We have a quote, count the next hashes.
                closing_hash_count = 0;
            }
            else if (scanner.branch(dsl::lit_c<'#'>))
            {
                // Count a hash if necessary.
                if (closing_hash_count != -1)
                    ++closing_hash_count;
            }
            else if (scanner.is_at_eof())
            {
                // Report an error for an unterminated literal.
                scanner.fatal_error(unterminated{}, scanner.begin(), scanner.position());
                return lexy::scan_failed;
            }
            else
            {
                // Parse any other code point, and invalidate hash count.
                scanner.parse(dsl::code_point);
                closing_hash_count = -1;
            }

            // Check for any errors we might have gotten.
            if (!scanner)
                return lexy::scan_failed;
        }
        auto content_end = scanner.position();

        return lexy::buffer_lexeme<lexy::utf8_encoding>(content_begin, content_end);
    }
};

