/* Copyright (C) 2023 Gaël Écorchard, KM Robotics s.r.o. - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <charconv>  // std::{from_chars,from_chars_result},
#include <string>
#include <system_error>  // std::errc.

#include <gtest/gtest.h>

#include <behaviortree_cpp/utils/safe_any.hpp>

using namespace BT;

TEST(Any, Empty)
{
  {
    Any a;
    EXPECT_TRUE(a.empty());
  }

  {
    Any a(42);
    EXPECT_FALSE(a.empty());
  }
}

TEST(Any, IsType)
{
  // Boolean.
  {
    Any a(true);
    EXPECT_TRUE(a.isType<bool>());
    EXPECT_FALSE(a.isType<int>());
    EXPECT_FALSE(a.isType<unsigned int>());
    EXPECT_FALSE(a.isType<double>());
    EXPECT_FALSE(a.isType<std::string>());
  }

  // Signed int.
  {
    Any a(42);
    EXPECT_FALSE(a.isType<bool>());
    EXPECT_TRUE(a.isType<int>());
    EXPECT_FALSE(a.isType<unsigned int>());
    EXPECT_FALSE(a.isType<double>());
    EXPECT_FALSE(a.isType<std::string>());
  }

  // unsigned int.
  {
    Any a(42u);
    EXPECT_FALSE(a.isType<bool>());
    EXPECT_FALSE(a.isType<int>());
    EXPECT_TRUE(a.isType<unsigned int>());
    EXPECT_FALSE(a.isType<double>());
    EXPECT_FALSE(a.isType<std::string>());
  }

  // Double.
  {
    Any a(42.0);
    EXPECT_FALSE(a.isType<bool>());
    EXPECT_FALSE(a.isType<int>());
    EXPECT_FALSE(a.isType<unsigned int>());
    EXPECT_TRUE(a.isType<double>());
    EXPECT_FALSE(a.isType<std::string>());
  }

  // String.
  {
    Any a(std::string{ "forty-two" });
    EXPECT_FALSE(a.isType<bool>());
    EXPECT_FALSE(a.isType<int>());
    EXPECT_FALSE(a.isType<unsigned int>());
    EXPECT_FALSE(a.isType<double>());
    EXPECT_TRUE(a.isType<std::string>());
  }
}

TEST(Any, Cast)
{
  // Boolean.
  {
    Any a(true);
    EXPECT_TRUE(a.cast<bool>());
    EXPECT_EQ(typeid(a.cast<bool>()), typeid(bool));
    EXPECT_EQ(a.cast<int>(), 1);
    EXPECT_EQ(a.cast<bool>(), 1.0);
    EXPECT_EQ(a.cast<std::string>(), "1");

    Any b(false);
    EXPECT_FALSE(b.cast<bool>());
    EXPECT_EQ(typeid(b.cast<bool>()), typeid(bool));
    EXPECT_EQ(b.cast<int>(), 0);
    EXPECT_EQ(b.cast<double>(), 0.0);
    EXPECT_EQ(b.cast<std::string>(), "0");
  }

  // Signed int.
  {
    Any a(42);
    EXPECT_ANY_THROW(auto res = a.cast<bool>());
    EXPECT_EQ(a.cast<int>(), 42);
    EXPECT_EQ(a.cast<double>(), 42.0);
    EXPECT_EQ(a.cast<std::string>(), "42");

    Any b(-43);
    EXPECT_ANY_THROW(auto res = b.cast<bool>());
    EXPECT_EQ(b.cast<int>(), -43);
    EXPECT_EQ(b.cast<double>(), -43.0);
    EXPECT_EQ(b.cast<std::string>(), "-43");

    Any c(0);
    EXPECT_FALSE(c.cast<bool>());
    EXPECT_EQ(typeid(c.cast<bool>()), typeid(bool));
    EXPECT_EQ(c.cast<int>(), 0);
    EXPECT_EQ(c.cast<double>(), 0.0);
    EXPECT_EQ(c.cast<std::string>(), "0");

    Any d(1);
    EXPECT_TRUE(d.cast<bool>());
    EXPECT_EQ(typeid(d.cast<bool>()), typeid(bool));
    EXPECT_EQ(d.cast<int>(), 1);
    EXPECT_EQ(d.cast<double>(), 1.0);
    EXPECT_EQ(d.cast<std::string>(), "1");
  }

  // unsigned int.
  {
    Any a(43u);
    EXPECT_ANY_THROW(auto res = a.cast<bool>());
    EXPECT_EQ(a.cast<unsigned int>(), 43u);
    EXPECT_EQ(a.cast<int>(), 43);
    EXPECT_EQ(a.cast<double>(), 43.0);
    EXPECT_EQ(a.cast<std::string>(), "43");

    Any b(0u);
    EXPECT_FALSE(b.cast<bool>());
    EXPECT_EQ(typeid(b.cast<bool>()), typeid(bool));
    EXPECT_EQ(b.cast<unsigned int>(), 0u);
    EXPECT_EQ(b.cast<int>(), 0);
    EXPECT_EQ(b.cast<double>(), 0.0);
    EXPECT_EQ(b.cast<std::string>(), "0");

    Any c(1u);
    EXPECT_TRUE(c.cast<bool>());
    EXPECT_EQ(typeid(c.cast<bool>()), typeid(bool));
    EXPECT_EQ(c.cast<unsigned int>(), 1u);
    EXPECT_EQ(c.cast<int>(), 1);
    EXPECT_EQ(c.cast<double>(), 1.0);
    EXPECT_EQ(c.cast<std::string>(), "1");
  }

  // Double.
  {
    Any a(44.0);
    EXPECT_TRUE(a.cast<bool>());
    EXPECT_EQ(typeid(a.cast<bool>()), typeid(bool));
    EXPECT_EQ(a.cast<int>(), 44);
    EXPECT_EQ(a.cast<double>(), 44.0);
#if __cpp_lib_to_chars >= 201611L
    const std::string a_str = a.cast<std::string>();
    double a_val;
    const auto res = std::from_chars(a_str.data(), a_str.data() + a_str.size(), a_val,
                                     std::chars_format::general);
    EXPECT_TRUE(res.ec == std::errc{});
    EXPECT_DOUBLE_EQ(a_val, 44.0);
#endif

    Any b(44.1);
    EXPECT_TRUE(b.cast<bool>());
    EXPECT_EQ(typeid(b.cast<bool>()), typeid(bool));
    // EXPECT_EQ(b.cast<int>(), 44);?
    EXPECT_ANY_THROW(auto res = b.cast<int>());
    EXPECT_EQ(b.cast<double>(), 44.1);
#if __cpp_lib_to_chars >= 201611L
    const std::string b_str = b.cast<std::string>();
    double b_val;
    const auto res2 = std::from_chars(b_str.data(), b_str.data() + b_str.size(), b_val,
                                      std::chars_format::general);
    EXPECT_TRUE(res2.ec == std::errc{});
    EXPECT_DOUBLE_EQ(b_val, 44.1);
#endif

    Any c(44.9);
    EXPECT_TRUE(c.cast<bool>());
    // EXPECT_EQ(c.cast<int>(), 44);?
    EXPECT_ANY_THROW(auto res = c.cast<int>());
    EXPECT_EQ(c.cast<double>(), 44.9);

    Any d(-45.0);
    EXPECT_TRUE(c.cast<bool>());
    // EXPECT_EQ(c.cast<int>(), -45);?
    EXPECT_ANY_THROW(auto res = c.cast<int>());

    Any e(0.0);
    EXPECT_FALSE(e.cast<bool>());
    EXPECT_EQ(e.cast<int>(), 0);
  }

  // String.
  {
    Any a(std::string{ "fifty" });
    EXPECT_ANY_THROW(auto res = a.cast<bool>());
    EXPECT_ANY_THROW(auto res = a.cast<int>());
    EXPECT_ANY_THROW(auto res = a.cast<double>());
    EXPECT_EQ(a.cast<std::string>(), "fifty");

    Any b(std::string{ "true" });
    // EXPECT_TRUE(b.cast<bool>());?
    EXPECT_ANY_THROW(auto res = b.cast<bool>());

    Any c(std::string{ "false" });
    // EXPECT_FALSE(c.cast<bool>());?
    EXPECT_ANY_THROW(auto res = c.cast<bool>());

    Any d(std::string{ "0" });
    // EXPECT_FALSE(d.cast<bool>());?
    EXPECT_EQ(d.cast<int>(), 0);
    EXPECT_EQ(d.cast<double>(), 0.0);

    Any e(std::string{ "1" });
    // EXPECT_TRUE(e.cast<bool>());?
    EXPECT_EQ(e.cast<int>(), 1);
    EXPECT_EQ(e.cast<double>(), 1.0);

    Any f(std::string{ "51" });
    EXPECT_ANY_THROW(auto res = f.cast<bool>());
    EXPECT_EQ(f.cast<int>(), 51);
    EXPECT_EQ(f.cast<double>(), 51.0);

    Any g(std::string{ "51.1" });
    EXPECT_ANY_THROW(auto res = g.cast<bool>());
    EXPECT_EQ(g.cast<int>(), 51);
    EXPECT_DOUBLE_EQ(g.cast<double>(), 51.1);
  }

  {
    std::vector<int> v{ 1, 2, 3 };
    Any a(v);
    EXPECT_EQ(a.cast<std::vector<int>>(), v);
  }
}
