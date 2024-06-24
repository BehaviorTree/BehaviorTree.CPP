#include <cstdio>
#include <lexy/input/argv_input.hpp>

//{
int main(int argc, char* argv[])
{
    auto begin = lexy::argv_begin(argc, argv);
    auto end   = lexy::argv_end(argc, argv);

    for (auto cur = begin; cur != end; ++cur)
    {
        if (*cur == '\0')
            std::puts("\\0");
        else
            std::printf("%c", *cur);
    }
    std::puts("\\0"); // last null terminator not included in the range
}
//}

