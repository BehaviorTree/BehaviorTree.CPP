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

#include "behaviortree_cpp/decorators/inverter_node.h"

namespace BT
{
InverterNode::InverterNode(const std::string& name) : DecoratorNode(name, {})
{
  setRegistrationID("Inverter");
}

NodeStatus InverterNode::tick()
{
  setStatus(NodeStatus::RUNNING);
  const NodeStatus child_status = child_node_->executeTick();

  switch(child_status)
  {
    case NodeStatus::SUCCESS: {
      resetChild();
      return NodeStatus::FAILURE;
    }

    case NodeStatus::FAILURE: {
      resetChild();
      return NodeStatus::SUCCESS;
    }

    case NodeStatus::RUNNING:
    case NodeStatus::SKIPPED: {
      return child_status;
    }

    case NodeStatus::IDLE: {
      throw LogicError("[", name(), "]: A children should not return IDLE");
    }
  }
  return status();
}

}  // namespace BT
