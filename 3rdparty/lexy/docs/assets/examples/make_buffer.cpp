#include <cstdio>
#include <list>

#include <lexy/action/match.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/buffer.hpp>

struct production
{
    static constexpr auto rule = LEXY_LIT("Hi");
};

struct file_span
{
    const void* memory;
    std::size_t size;
};

file_span map_file(const char* /* path */)
{
    // fake something
    static constexpr unsigned char memory[] = {'H', 0x00, 'i', 0x00};
    return {memory, 4};
}

//{
int main()
{
    // Map a file into memory.
    auto span = map_file("input.txt");

    // Treat the file as little endian UTF-16.
    constexpr auto make_utf16_le
        = lexy::make_buffer_from_raw<lexy::utf16_encoding, lexy::encoding_endianness::little>;
    auto input = make_utf16_le(span.memory, span.size);

    // Use the input.
    if (!lexy::match<production>(input))
    {
        std::puts("Error!\n");
        return 1;
    }
}
//}

