/*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/behavior_tree.h"
#include <cstring>

namespace BT
{
void applyRecursiveVisitor(const TreeNode* node,
                           const std::function<void(const TreeNode*)>& visitor)
{
  if(!node)
  {
    throw LogicError("One of the children of a DecoratorNode or ControlNode is nullptr");
  }

  visitor(node);

  if(auto control = dynamic_cast<const BT::ControlNode*>(node))
  {
    for(const auto& child : control->children())
    {
      applyRecursiveVisitor(static_cast<const TreeNode*>(child), visitor);
    }
  }
  else if(auto decorator = dynamic_cast<const BT::DecoratorNode*>(node))
  {
    applyRecursiveVisitor(decorator->child(), visitor);
  }
}

void applyRecursiveVisitor(TreeNode* node, const std::function<void(TreeNode*)>& visitor)
{
  if(!node)
  {
    throw LogicError("One of the children of a DecoratorNode or ControlNode is nullptr");
  }

  visitor(node);

  if(auto control = dynamic_cast<BT::ControlNode*>(node))
  {
    for(const auto& child : control->children())
    {
      applyRecursiveVisitor(child, visitor);
    }
  }
  else if(auto decorator = dynamic_cast<BT::DecoratorNode*>(node))
  {
    if(decorator->child())
    {
      applyRecursiveVisitor(decorator->child(), visitor);
    }
  }
}

void printTreeRecursively(const TreeNode* root_node, std::ostream& stream)
{
  std::function<void(unsigned, const BT::TreeNode*)> recursivePrint;

  recursivePrint = [&recursivePrint, &stream](unsigned indent, const BT::TreeNode* node) {
    for(unsigned i = 0; i < indent; i++)
    {
      stream << "   ";
    }
    if(!node)
    {
      stream << "!nullptr!" << std::endl;
      return;
    }
    stream << node->name() << std::endl;
    indent++;

    if(auto control = dynamic_cast<const BT::ControlNode*>(node))
    {
      for(const auto& child : control->children())
      {
        recursivePrint(indent, child);
      }
    }
    else if(auto decorator = dynamic_cast<const BT::DecoratorNode*>(node))
    {
      recursivePrint(indent, decorator->child());
    }
  };

  stream << "----------------" << std::endl;
  recursivePrint(0, root_node);
  stream << "----------------" << std::endl;
}

void buildSerializedStatusSnapshot(TreeNode* root_node,
                                   SerializedTreeStatus& serialized_buffer)
{
  serialized_buffer.clear();

  auto visitor = [&serialized_buffer](const TreeNode* node) {
    serialized_buffer.push_back(
        std::make_pair(node->UID(), static_cast<uint8_t>(node->status())));
  };

  applyRecursiveVisitor(root_node, visitor);
}

int LibraryVersionNumber()
{
  static int number = -1;
  if(number == -1)
  {
    auto const parts = splitString(BTCPP_LIBRARY_VERSION, '.');
    number = std::stoi(std::string(parts[0])) * 10000 +
             std::stoi(std::string(parts[1])) * 100 + std::stoi(std::string(parts[2]));
  }
  return number;
}

const char* LibraryVersionString()
{
  return BTCPP_LIBRARY_VERSION;
}

}  // namespace BT
