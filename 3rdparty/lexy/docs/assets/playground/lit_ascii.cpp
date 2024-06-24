// INPUT:Hello World!
struct production
{
    // The character type doesn't matter if it only contains ASCII characters.
    // The literal is encoded in UTF-16 whereas the (playground) input
    // is encoded in UTF-8, but as its only ASCII characters,
    // lexy will transcode for you.
    static constexpr auto rule = LEXY_LIT(u"Hello World!");
};
