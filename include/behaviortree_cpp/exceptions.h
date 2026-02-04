/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2025 Davide Faconti, Eurecat -  All Rights Reserved
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

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

/// Information about a node in the tick backtrace.
/// Uses string_view since these reference strings owned by TreeNode,
/// which outlives the exception in normal usage.
struct TickBacktraceEntry
{
  std::string_view node_name;
  std::string_view node_path;
  std::string_view registration_name;
};

/// Exception thrown when a node's tick() method throws an exception.
/// Contains the originating node and full tick backtrace showing the path through the tree.
class NodeExecutionError : public RuntimeError
{
public:
  NodeExecutionError(std::vector<TickBacktraceEntry> backtrace,
                     const std::string& original_message)
    : RuntimeError(formatMessage(backtrace, original_message))
    , backtrace_(std::move(backtrace))
    , original_message_(original_message)
  {}

  /// The node that threw the exception (innermost in the backtrace)
  [[nodiscard]] const TickBacktraceEntry& failedNode() const
  {
    return backtrace_.back();
  }

  /// Full tick backtrace from root to failing node
  [[nodiscard]] const std::vector<TickBacktraceEntry>& backtrace() const
  {
    return backtrace_;
  }

  [[nodiscard]] const std::string& originalMessage() const
  {
    return original_message_;
  }

private:
  std::vector<TickBacktraceEntry> backtrace_;
  std::string original_message_;

  static std::string formatMessage(const std::vector<TickBacktraceEntry>& bt,
                                   const std::string& original_msg)
  {
    std::string msg =
        StrCat("Exception in node '", bt.back().node_path, "': ", original_msg);
    msg += "\nTick backtrace:";
    for(size_t i = 0; i < bt.size(); ++i)
    {
      const bool is_last = (i == bt.size() - 1);
      msg += StrCat("\n  ", is_last ? "-> " : "   ", bt[i].node_path, " (",
                    bt[i].registration_name, ")");
    }
    return msg;
  }
};

}  // namespace BT

#endif
