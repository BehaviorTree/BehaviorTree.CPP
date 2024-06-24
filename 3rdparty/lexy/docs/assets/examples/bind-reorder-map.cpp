#include <cstdio>
#include <lexy/callback.hpp>

//{
constexpr auto my_callback = lexy::callback<int>([](int a, int b) { return a - b; });

// Swap the arguments...
constexpr auto bound = lexy::bind(my_callback, lexy::_2,
                                  // ... and double the (old) first one.
                                  lexy::_1.map([](int i) { return 2 * i; }));
//}

int main()
{
    std::printf("result: %d\n", bound(11, 42)); // 42 - 22
}

