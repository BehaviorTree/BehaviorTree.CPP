/* Copyright (C) 2025 Davide Faconti -  All Rights Reserved
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy of
 *   this software and associated documentation files (the "Software"), to deal in
 *   the Software without restriction, including without limitation the rights to
 *   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *   the Software, and to permit persons to whom the Software is furnished to do so,
 *   subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "behaviortree_cpp/control_node.h"

namespace BT
{
/**
 * @brief Common base of the control nodes that tick their children in order and stop
 * at the first one that returns RUNNING (ReactiveSequence, ReactiveFallback).
 *
 * It exists so that a node outside the reactive node can ask it to let go of the
 * branches it is running without knowing which of the two it is.
 *
 * Deliberately not virtual: the class adds state and one non-virtual accessor only,
 * so no vtable of an existing node changes.
 */
class ReactiveControlNode : public ControlNode
{
public:
  ReactiveControlNode(const std::string& name, const NodeConfig& config)
    : ControlNode(name, config)
  {}

  /** Halt every child that is RUNNING, except the one the caller is inside.
   *
   * Used by Preempt: the ancestor cannot tell by index which branch has to go,
   * because the index is only updated by its own tick(), and the halting happens
   * below it. Asking the children for their status cannot go stale.
   */
  void haltRunningChild(const TreeNode* excluded)
  {
    running_child_ = -1;
    for(size_t i = 0; i < childrenCount(); i++)
    {
      if(children()[i] != excluded && children()[i]->status() == NodeStatus::RUNNING)
      {
        haltChild(i);
      }
    }
  }

protected:
  int running_child_ = -1;
};

}  // namespace BT
