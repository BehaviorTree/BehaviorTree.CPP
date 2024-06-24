#include <cstdio>

#include <lexy/action/match.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

struct production
{
    static constexpr auto rule = LEXY_LIT("Hi");
};

//{
int main()
{
    unsigned char array[] = {'H', 'i'};

    // Create the input, deducing the encoding.
    auto input = lexy::string_input(array, array + 2);

    // Use the input.
    if (!lexy::match<production>(input))
    {
        std::puts("Error!\n");
        return 1;
    }
}
//}

