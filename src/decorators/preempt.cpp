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

#include "behaviortree_cpp/decorators/preempt.h"

#include "behaviortree_cpp/controls/reactive_control_node.h"

namespace BT
{
NodeStatus Preempt::tick()
{
  // A new branch starts now: let go of whatever the reactive ancestors are
  // still running before the child below us begins. status() is IDLE again
  // whenever a parent resets a completed node, so this also covers a second
  // preemption after the child returned SUCCESS or FAILURE.
  if(status() == NodeStatus::IDLE)
  {
    haltPreviousBranch();
  }
  return child()->executeTick();
}

void Preempt::haltPreviousBranch()
{
  // Walk up the tree keeping track of the child we came from: a reactive
  // ancestor is only asked to let go of the children that are not on our path,
  // so it never halts the branch we are about to start.
  TreeNode* child_on_path = this;

  for(TreeNode* ancestor = parent(); ancestor != nullptr; ancestor = ancestor->parent())
  {
    if(auto* reactive = dynamic_cast<ReactiveControlNode*>(ancestor))
    {
      reactive->haltRunningChild(child_on_path);
    }
    child_on_path = ancestor;
  }
}

}  // namespace BT
