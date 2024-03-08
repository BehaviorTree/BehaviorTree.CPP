/*  Copyright (C) 2022 Davide Faconti -  All Rights Reserved
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

#pragma once

#include "behaviortree_cpp/blackboard.h"

namespace BT
{

/// Simple map (string->nt), used to convert enums in the
/// scripting language
using EnumsTable = std::unordered_map<std::string, int>;
using EnumsTablePtr = std::shared_ptr<EnumsTable>;

namespace Ast
{
/**
   * @brief The Environment class is used to encapsulate
   * the information and states needed by the scripting language
   */
struct Environment
{
  BT::Blackboard::Ptr vars;
  EnumsTablePtr enums;
};
}  // namespace Ast

/**
 * @brief ValidateScript will check if a certain string is valid.
 */
Result ValidateScript(const std::string& script);

using ScriptFunction = std::function<Any(Ast::Environment& env)>;

Expected<ScriptFunction> ParseScript(const std::string& script);

Expected<Any> ParseScriptAndExecute(Ast::Environment& env, const std::string& script);

}  // namespace BT
