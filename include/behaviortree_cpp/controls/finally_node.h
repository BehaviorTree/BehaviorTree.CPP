#pragma once

#include "behaviortree_cpp/control_node.h"

#include <exception>

namespace BT
{
/**
 * @brief The Finally node ticks its first child ("main") and then ticks its
 * second child ("cleanup"), like try/finally.
 *
 * - Cleanup runs after main returns SUCCESS, FAILURE or SKIPPED.
 * - If main throws (any type), main is halted, cleanup runs, and then the
 *   exception is rethrown.
 * - The node returns main's status, or FAILURE if cleanup fails.
 * - If this node is halted while main or cleanup is RUNNING, main is halted
 *   and cleanup is ticked again every 10 ms on the thread calling halt(), so
 *   asynchronous cleanup can finish. This stops when cleanup finishes, fails
 *   or throws, or when "halt_timeout_msec" runs out, in which case cleanup is
 *   halted unfinished. Cleanup always gets at least one tick, no later tick
 *   starts after the timeout, and a tick that blocks is not cut short. halt() blocks its caller, and any lock the
 *   caller holds, until then, so a slow cleanup also delays a parent such as
 *   a ReactiveSequence. Call halt() from the thread that ticks the tree.
 * - An exception thrown by cleanup in tick() halts cleanup, leaves the node
 *   IDLE and propagates. It replaces main's status, or main's exception,
 *   which is dropped. The next tick starts with main.
 * - halt() never throws, because it also runs from ~Tree(). It prints to
 *   stderr when cleanup throws, fails or times out, and when halting a child
 *   throws. A pending exception from main is dropped by a halt.
 *
 * Requires exactly 2 children, checked when the XML is loaded and on tick.
 */
class FinallyNode : public ControlNode
{
public:
  FinallyNode(const std::string& name, const NodeConfig& config);

  ~FinallyNode() override = default;

  FinallyNode(const FinallyNode&) = delete;
  FinallyNode& operator=(const FinallyNode&) = delete;
  FinallyNode(FinallyNode&&) = delete;
  FinallyNode& operator=(FinallyNode&&) = delete;

  static PortsList providedPorts()
  {
    return { InputPort<unsigned>("halt_timeout_msec", kDefaultHaltTimeoutMsec,
                                 "When halted, how long to keep ticking cleanup "
                                 "before halting it unfinished, in milliseconds") };
  }

  void halt() override;

private:
  static constexpr unsigned kDefaultHaltTimeoutMsec = 10000;

  bool in_cleanup_ = false;
  NodeStatus main_status_ = NodeStatus::IDLE;
  std::exception_ptr main_exception_;

  void haltChildNoThrow(size_t i);
  void finishCleanupDuringHalt();

  BT::NodeStatus tick() override;
};

}  // namespace BT
