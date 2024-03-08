/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#ifndef BT_EXCEPTIONS_H
#define BT_EXCEPTIONS_H

#include <string>
#include <stdexcept>
#include "utils/strcat.hpp"

namespace BT
{
class BehaviorTreeException : public std::exception
{
public:
  BehaviorTreeException(std::string_view message)
    : message_(static_cast<std::string>(message))
  {}

  template <typename... SV>
  BehaviorTreeException(const SV&... args) : message_(StrCat(args...))
  {}

  const char* what() const noexcept
  {
    return message_.c_str();
  }

private:
  std::string message_;
};

// This errors are usually related to problems which "probably" require code refactoring
// to be fixed.
class LogicError : public BehaviorTreeException
{
public:
  LogicError(std::string_view message) : BehaviorTreeException(message)
  {}

  template <typename... SV>
  LogicError(const SV&... args) : BehaviorTreeException(args...)
  {}
};

// This errors are usually related to problems that are relted to data or conditions
// that happen only at run-time
class RuntimeError : public BehaviorTreeException
{
public:
  RuntimeError(std::string_view message) : BehaviorTreeException(message)
  {}

  template <typename... SV>
  RuntimeError(const SV&... args) : BehaviorTreeException(args...)
  {}
};

}  // namespace BT

#endif
