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

#include "behaviortree_cpp/decorator_node.h"

namespace BT
{
/**
 * @brief Marks the point where a new branch of the tree starts.
 *
 * A reactive node knows how to halt the siblings of the child that returned
 * RUNNING, but it does so after ticking that child. When a check that returns
 * SUCCESS immediately and a new action that will keep running look the same to
 * it, the tree cannot tell "start a new branch" from "re-tick a check", and the
 * previously running branch is left RUNNING while the new one starts (issue
 * #1031).
 *
 * Preempt answers that question at the one place that knows the answer: the tree
 * author, who wraps the action that starts the new branch.
 *
 *   <ReactiveSequence>
 *     <Fallback>
 *       <BatteryOk/>
 *       <Preempt><GotoCharging/></Preempt>
 *     </Fallback>
 *     <GotoKitchen/>
 *   </ReactiveSequence>
 *
 * On its first tick, before ticking its child, it halts the RUNNING child of
 * every reactive ancestor it is not itself inside. Children of a Parallel are
 * left alone: that node is allowed to keep several of them RUNNING, so halting
 * them because a sibling branch moved on would be wrong.
 *
 * The name is a placeholder.
 */
class Preempt : public DecoratorNode
{
public:
  Preempt(const std::string& name, const NodeConfig& config) : DecoratorNode(name, config)
  {
    setRegistrationID("Preempt");
  }

  ~Preempt() override = default;

  static BT::PortsList providedPorts()
  {
    return {};
  }

  Preempt(const Preempt&) = delete;
  Preempt& operator=(const Preempt&) = delete;
  Preempt(Preempt&&) = delete;
  Preempt& operator=(Preempt&&) = delete;

private:
  NodeStatus tick() override;

  void halt() override;

  void haltPreviousBranch();

  bool started_ = false;
};

}  // namespace BT
