#pragma once

#include "behaviortree_cpp/control_node.h"

#include <atomic>
#include <exception>
#include <thread>

namespace BT
{
/**
 * @brief The Finally node ticks its first child ("main") and then ticks its
 * second child ("cleanup"), like try/finally.
 *
 * - Cleanup runs after main returns SUCCESS or FAILURE. If main returns SKIPPED on
 *   its first tick it did nothing, so cleanup is not run and the node returns
 *   SKIPPED. If main is skipped after it returned RUNNING, for example by _while,
 *   cleanup runs and the node returns SKIPPED.
 * - If main throws (any type), main is halted, cleanup runs, and then the
 *   exception is rethrown.
 * - The node returns main's status, or FAILURE if cleanup fails.
 * - If this node is halted while main or cleanup is RUNNING, main is halted and
 *   cleanup gets one tick, like TryCatch with catch_on_halt="true". A cleanup that
 *   needs several ticks is cut short by the final halt of its subtree. That tick
 *   runs inside halt(), on the calling thread and under any lock the caller holds,
 *   so a cleanup action that blocks also blocks the caller.
 * - The single tick suits synchronous cleanup. An asynchronous cleanup child (e.g. a
 *   StatefulActionNode) gets onStart() and then onHalted() right away. Use
 *   "halt_timeout_msec" for asynchronous cleanup on halt.
 * - To let a multi-step cleanup finish, set "halt_timeout_msec" above 10 (the tick
 *   period). Then halt() ticks cleanup once, and again every 10 ms until it finishes,
 *   fails or throws, or the timeout runs out, in which case cleanup is halted
 *   unfinished. No tick starts after the timeout, and a tick that blocks is not cut
 *   short. With 10 or less, cleanup gets a single tick.
 * - That loop BLOCKS the halting parent, e.g. a ReactiveSequence, for up to
 *   halt_timeout_msec, and nothing else in the tree is ticked meanwhile. An
 *   enclosing Timeout that holds its mutex during the child's tick is delayed by up
 *   to the same amount, and nested Finally nodes with the option set add up their
 *   timeouts. Keep the value short.
 * - The loop runs only when halt() is called from the thread that last ticked this
 *   node. A halt from another thread, such as a Timeout decorator's timer or a
 *   service callback calling Tree::haltTree(), gives cleanup the single tick and
 *   prints a note to stderr. A Finally nested inside that halt also gets a single
 *   tick. If halt() is called while another halt() of this node is still running,
 *   it returns at once and leaves the cleanup to the first one. halt() is not
 *   synchronized with a concurrent tick().
 * - An exception thrown by cleanup in tick() halts cleanup, leaves the node
 *   IDLE and propagates. It replaces main's status, or main's exception,
 *   which is dropped. The next tick starts with main.
 * - halt() never throws, because it also runs from ~Tree(). It prints to
 *   stderr when cleanup throws, fails or times out, and when halting a child
 *   throws. A pending exception from main is printed and dropped by a halt.
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
    return { InputPort<unsigned>(
        "halt_timeout_msec", kDefaultHaltTimeoutMsec,
        "When halted, how long to keep ticking cleanup before halting it "
        "unfinished, in milliseconds. 10 (the tick period) or less: a single "
        "tick, which cuts an asynchronous cleanup short. Above 10, halt() blocks "
        "the halting parent, and any enclosing Timeout, for up to this long, and "
        "nested Finally nodes add up; halts from another thread get a single "
        "tick. Keep it short") };
  }

  void halt() noexcept override;

private:
  static constexpr unsigned kDefaultHaltTimeoutMsec = 0;

  // Written in tick(), read in halt(), which may run on another thread.
  std::atomic<std::thread::id> tick_thread_{};
  // Set while a halt() runs, so a concurrent halt() from another thread does not tick
  // cleanup at the same time.
  std::atomic<bool> halting_{ false };
  bool in_cleanup_ = false;
  bool main_started_ = false;
  NodeStatus main_status_ = NodeStatus::IDLE;
  std::exception_ptr main_exception_;

  void haltChildNoThrow(size_t i) noexcept;
  void finishCleanupDuringHalt(bool foreign_thread);

  BT::NodeStatus tick() override;
};

}  // namespace BT
