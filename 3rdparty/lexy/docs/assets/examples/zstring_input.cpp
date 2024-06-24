#include <cstdio>
#include <list>

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
    // Create the input, deducing the encoding.
    auto input = lexy::zstring_input("Hi");

    // Use the input.
    if (!lexy::match<production>(input))
    {
        std::puts("Error!\n");
        return 1;
    }
}
//}

