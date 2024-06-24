#include <cstdio>
#include <lexy/callback/adapter.hpp>

//{
constexpr auto my_callback
    // The return type is int.
    = lexy::callback<int>([](int i) { return 2 * i; }, //
                          [](int a, int b) { return a + b; });
//}

int main()
{
    std::printf("one argument: %d\n", my_callback(11));
    std::printf("two arguments: %d\n", my_callback(11, 42));
}

