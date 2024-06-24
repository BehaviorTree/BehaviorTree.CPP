#include <cstdio>
#include <list>

#include <lexy/action/match.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/range_input.hpp>

struct production
{
    static constexpr auto rule = LEXY_LIT("Hi");
};

//{
int main()
{
    std::list<char16_t> list{u'H', u'i'};

    // Create the input, deducing the encoding.
    auto input = lexy::range_input(list.begin(), list.end());

    // Use the input.
    if (!lexy::match<production>(input))
    {
        std::puts("Error!\n");
        return 1;
    }
}
//}

