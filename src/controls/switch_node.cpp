/* Copyright (C) 2019-2022 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/controls/switch_node.h"

#if __has_include(<charconv>)
#include <charconv>
#endif

namespace BT::details
{

bool CheckStringEquality(const std::string& v1, const std::string& v2,
                         const ScriptingEnumsRegistry* enums)
{
  // compare strings first
  if(v1 == v2)
  {
    return true;
  }
  // compare as integers next
  auto ToInt = [enums](const std::string& str, auto& result) -> bool {
    if(enums)
    {
      auto it = enums->find(str);
      if(it != enums->end())
      {
        result = it->second;
        return true;
      }
    }
#if __cpp_lib_to_chars >= 201611L
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    return (ec == std::errc());
#else
    try
    {
      result = std::stoi(str);
      return true;
    }
    catch(...)
    {
      return false;
    }
#endif
  };
  int v1_int = 0;
  int v2_int = 0;
  if(ToInt(v1, v1_int) && ToInt(v2, v2_int) && v1_int == v2_int)
  {
    return true;
  }
  // compare as real numbers next
  auto ToReal = [](const std::string& str, auto& result) -> bool {
#if __cpp_lib_to_chars >= 201611L
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    return (ec == std::errc());
#else
    try
    {
      result = std::stod(str);
      return true;
    }
    catch(...)
    {
      return false;
    }
#endif
  };
  double v1_real = 0;
  double v2_real = 0;
  constexpr auto eps = double(std::numeric_limits<float>::epsilon());
  if(ToReal(v1, v1_real) && ToReal(v2, v2_real) && std::abs(v1_real - v2_real) <= eps)
  {
    return true;
  }
  return false;
}

}  // namespace BT::details
