#include <cstdio>
#include <lexy/callback.hpp>
#include <string>

//{
constexpr auto my_strlen
    // Construct a string, then return its size.
    = lexy::as_string<std::string> | lexy::callback<std::size_t>(&std::string::size);
//}

int main()
{
    std::printf("length: %zu\n", my_strlen("Hello"));
}

