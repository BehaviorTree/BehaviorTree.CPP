#include <cstdio>
#include <lexy/callback.hpp>

//{
constexpr auto my_callback
    = lexy::callback<int>([](int factor, int i) { return factor * i; },
                          [](int factor, int a, int b) { return factor * (a + b); });

// Bind the first parameter and forward the rest unchanged.
constexpr auto bound = lexy::bind(my_callback, 2, lexy::values);
//}

int main()
{
    std::printf("one argument: %d\n", bound(11));      // 2 * 11
    std::printf("two arguments: %d\n", bound(11, 42)); // 2 * (11 + 42)
}

