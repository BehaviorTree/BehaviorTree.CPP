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
 * It exists so that a node outside the reactive node can ask it to let go of its
 * running child without knowing which of the two it is, and so that the index stays
 * consistent with the child that is actually RUNNING.
 *
 * Deliberately not virtual: the class adds state and two non-virtual accessors only,
 * so no vtable of an existing node changes.
 */
class ReactiveControlNode : public ControlNode
{
public:
  ReactiveControlNode(const std::string& name, const NodeConfig& config)
    : ControlNode(name, config)
  {}

  /// Index of the child that returned RUNNING, or -1 if there is none.
  [[nodiscard]] int runningChild() const
  {
    return running_child_;
  }

  /** Halt the child that is currently RUNNING, if any, and forget it.
   *
   * Used by Preempt: the reactive ancestor cannot halt that child on its own,
   * because from its point of view the child is still the one it has to tick.
   */
  void haltRunningChild()
  {
    if(running_child_ >= 0 && static_cast<size_t>(running_child_) < childrenCount())
    {
      haltChild(static_cast<size_t>(running_child_));
    }
    running_child_ = -1;
  }

protected:
  int running_child_ = -1;
};

}  // namespace BT
