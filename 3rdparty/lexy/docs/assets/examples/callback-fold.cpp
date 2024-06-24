#include <cstdio>
#include <functional>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/fold.hpp>

//{
constexpr auto my_callback = lexy::callback(lexy::fold<int>(0, std::plus<int>{}));
//}

int main()
{
    std::printf("sum: %d\n", my_callback(1, 2, 3));
}

