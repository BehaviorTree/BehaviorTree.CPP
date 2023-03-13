[![language.badge]][language.url] [![standard.badge]][standard.url] [![license.badge]][license.url] [![travis.badge]][travis.url] [![appveyor.badge]][appveyor.url] [![release.badge]][release.url] [![godbolt.badge]][godbolt.url] [![wandbox.badge]][wandbox.url]

# Wildcards

*Wildcards* is a simple C++ header-only template library which implements
a general purpose algorithm for matching using wildcards. It supports both
runtime and compile time execution.

## Basic usage

The following examples of the basic usage are functionaly equivalent.

```C++
#include <wildcards.hpp>

int main()
{
  {
    using wildcards::match;

    static_assert(match("Hello, World!", "H*World?"), "");
  }

  {
    using wildcards::make_matcher;

    static_assert(make_matcher("H*World?").matches("Hello, World!"), "");
  }

  {
    using namespace wildcards::literals;

    static_assert("H*World?"_wc.matches("Hello, World!"), "");
  }

  return 0;
}
```

## Advanced usage

The following examples of the advanced usage are functionaly equivalent.

```C++
#include <wildcards.hpp>

int main()
{
  {
    using wildcards::match;

    static_assert(match("Hello, World!", "H%World_", {'%', '_', '\\'}), "");
  }

  {
    using wildcards::make_matcher;

    static_assert(make_matcher("H%World_", {'%', '_', '\\'}).matches("Hello, World!"), "");
  }

  return 0;
}
```

See more useful and complex [examples](example) and try them online! See also
[the tests](test/src/wildcards) to learn more.

## Demonstration on Compiler Explorer

Check compilers output of the following example on [Compiler Explorer][godbolt.url].

```C++
#include <wildcards.hpp>

using namespace wildcards::literals;

constexpr auto pattern = "*.[hc](pp|)"_wc;

// returns true
bool test1()
{
  constexpr auto res = pattern.matches("source.c");

  static_assert(res, "must be true");

  return res;
}

// returns false
bool test2()
{
  constexpr auto res = pattern.matches("source.cc");

  static_assert(!res, "must be false");

  return res;
}
```

## Integration

1. Single-header approach
   * Copy [`wildcards.hpp`](single_include/wildcards.hpp) from
     [`single_include`](single_include) directory to your project's header
     search path.
   * Add `#include <wildcards.hpp>` to your source file.
   * Use `wildcards::match()` or `wildcards::make_matcher()`. You can also use
     operator `""_wc` from `wildcards::literals` namespace.

2. Multi-header approach
   * Add [`include`](include) directory to your project's header search path.
   * Add `#include <wildcards.hpp>` to your source file.
   * Use `wildcards::match()` or `wildcards::make_matcher()`. You can also use
     operator `""_wc` from `wildcards::literals` namespace.

## Portability

The library requires at least a C++11 compiler to build. It has no external
dependencies.

The following compilers are continuously tested at [Travis CI][travis.url]
and [Appveyor CI][appveyor.url].

| Compiler            | Version | Operating System    | Notes                   |
|---------------------|---------|---------------------|-------------------------|
| Xcode               | 9.0     | OS X 10.12          | C++11/14/17             |
| Clang (with libcxx) | 3.9     | Ubuntu 14.04 LTS    | C++14/17                |
| Clang (with libcxx) | 4.0     | Ubuntu 14.04 LTS    | C++11/14/17             |
| Clang (with libcxx) | 5.0     | Ubuntu 14.04 LTS    | C++11/14/17             |
| Clang (with libcxx) | 6.0     | Ubuntu 14.04 LTS    | C++11/14/17             |
| GCC                 | 5.5     | Ubuntu 14.04 LTS    | C++11/14/17             |
| GCC                 | 6.4     | Ubuntu 14.04 LTS    | C++11/14/17             |
| GCC                 | 7.3     | Ubuntu 14.04 LTS    | C++11/14/17             |
| GCC                 | 8.1     | Ubuntu 14.04 LTS    | C++11/14/17             |
| DJGPP               | 7.2     | Ubuntu 14.04 LTS    | C++11/14/17, build only |
| Visual Studio       | 14 2015 | Windows Server 2016 | C++11/14/17, limited    |
| Visual Studio       | 15 2017 | Windows Server 2016 | C++11/14/17             |
| MinGW               | 6.3     | Windows Server 2016 | C++11/14/17             |
| MinGW               | 7.2     | Windows Server 2016 | C++11/14/17             |
| MinGW               | 7.3     | Windows Server 2016 | C++11/14/17             |

## License

This project is licensed under the [Boost 1.0][license.url].

## Details

### Syntax

| Pattern   | Meaning                                        |
| --------- | ---------------------------------------------- |
| `*`       | Matches everything.                            |
| `?`       | Matches any single character.                  |
| `\`       | Escape character.                              |
| `[abc]`   | Matches any character in *Set*.                |
| `[!abc]`  | Matches any character not in *Set*.            |
| `(ab\|c)` | Matches one of the sequences in *Alternative*. |

* *Set* cannot be empty. Any special character loses its special meaning in it.
* *Alternative* can contain more than two or just one sequence.
* The use of *Sets* and *Alternatives* can be switched off.
* Special characters are predefined for `char`, `char16_t`, `char32_t`
  and `wchar_t`, but can be redefined.

### Technical Notes

* *Wildcards* depends on two components which originate from external sources
  and were made part of the repository:
  * [`cpp_feature.hpp`](include/cpp_feature.hpp) taken from
    [here](https://github.com/ned14/quickcpplib/blob/master/include/cpp_feature.h),
  * [`catch.hpp`](test/include/catch.hpp) taken from
    [here](https://github.com/catchorg/Catch2/releases/download/v2.4.2/catch.hpp).

* *Wildcards* uses a recursive approach. Hence you can simply run out of stack
  (during runtime execution) or you can exceed the maximum depth of constexpr
  evaluation (during compile time execution). If so, try making the input
  sequence shorter or the pattern less complex. You can also try to build using
  the C++14 standard since the C++14 implementation of the library is more
  effective and consumes less resources.

* Place more specific sequences in *Alternatives* first. This becomes important
  when *Alternatives* are nested. E.g. `match("source.cpp", "(*.[hc](pp|))")`
  will work as expected but `match("source.cpp", "(*.[hc](|pp))")` will not.
  Fixing that would make *Wildcards* unreasonably complex.

* The `cx` library is a byproduct created during the development of *Wildcards*
  which uses some pieces from its functionality internally. More of the `cx` is
  used in tests and examples. You can use this library in exactly the same way
  as you use *Wildcards* (single-header / multi-header approach) but if you are
  interested only in *Wildcards*, you don't need to care about the `cx` at all.
  This library might become a separate project in the future.

[language.url]:   https://isocpp.org/
[language.badge]: https://img.shields.io/badge/language-C++-blue.svg

[standard.url]:   https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[standard.badge]: https://img.shields.io/badge/C%2B%2B-11%2F14%2F17-blue.svg

[license.url]:    http://www.boost.org/LICENSE_1_0.txt
[license.badge]:  https://img.shields.io/badge/license-Boost%201.0-blue.svg

[travis.url]:     https://travis-ci.org/zemasoft/wildcards
[travis.badge]:   https://travis-ci.org/zemasoft/wildcards.svg?branch=master

[appveyor.url]:   https://ci.appveyor.com/project/zemasoft/wildcards
[appveyor.badge]: https://ci.appveyor.com/api/projects/status/github/zemasoft/wildcards?svg=true&branch=master

[release.url]:    https://github.com/zemasoft/wildcards/releases
[release.badge]:  https://img.shields.io/github/release/zemasoft/wildcards.svg

[godbolt.url]:    https://godbolt.org/z/nPr4h7
[godbolt.badge]:  https://img.shields.io/badge/try%20it-on%20godbolt-blue.svg

[wandbox.url]:    https://github.com/zemasoft/wildcards/tree/master/example
[wandbox.badge]:  https://img.shields.io/badge/try%20it-on%20wandbox-blue.svg
