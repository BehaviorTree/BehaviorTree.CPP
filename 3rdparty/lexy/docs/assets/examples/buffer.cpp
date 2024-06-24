#include <cstdio>
#include <list>

#include <lexy/action/match.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/buffer.hpp>

struct production
{
    static constexpr auto rule = LEXY_LIT("Hi");
};

//{
int main()
{
    // Create a buffered input.
    auto input = [] {
        lexy::buffer<lexy::utf8_encoding>::builder builder(2);
        builder.data()[0] = 'H';
        builder.data()[1] = 'i';
        return std::move(builder).finish();
    }();

    // Use the input.
    if (!lexy::match<production>(input))
    {
        std::puts("Error!\n");
        return 1;
    }
}
//}

