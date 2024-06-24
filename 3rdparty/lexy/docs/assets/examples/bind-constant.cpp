#include <cstdio>
#include <lexy/callback.hpp>

//{
constexpr auto my_callback
    = lexy::callback<int>([](int factor, int i) { return factor * i; },
                          [](int factor, int a, int b) { return factor * (a + b); });

// Bind all arguments.
constexpr auto bound = lexy::bind(my_callback, 2, 11);
//}

int main()
{
    std::printf("zero arguments: %d\n", bound()); // 2 * 11
    std::printf("one argument: %d\n", bound(42)); // 2 * 11
}

