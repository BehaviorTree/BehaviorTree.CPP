#include "behaviortree_cpp/controls/finally_node.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <string_view>
#include <thread>
#include <utility>

namespace BT
{
namespace
{
constexpr std::chrono::milliseconds kHaltTickPeriod{ 10 };

// True while this thread runs a halt() that came from a thread other than the one ticking the
// node. A Finally ticked or halted inside it, e.g. as cleanup, must not run its blocking loop.
thread_local bool tls_in_foreign_halt = false;

class ForeignHaltScope
{
public:
  explicit ForeignHaltScope(bool foreign)
    : previous_(std::exchange(tls_in_foreign_halt, tls_in_foreign_halt || foreign))
  {}
  ~ForeignHaltScope()
  {
    tls_in_foreign_halt = previous_;
  }
  ForeignHaltScope(const ForeignHaltScope&) = delete;
  ForeignHaltScope& operator=(const ForeignHaltScope&) = delete;
  ForeignHaltScope(ForeignHaltScope&&) = delete;
  ForeignHaltScope& operator=(ForeignHaltScope&&) = delete;

private:
  bool previous_;
};

void printException(std::string_view node_name, const char* context,
                    const std::exception_ptr& exception) noexcept
{
  std::cerr << "[" << node_name << "]: " << context << ": ";
  try
  {
    std::rethrow_exception(exception);
  }
  catch(const std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "non-std exception" << std::endl;
  }
}
}  // namespace

FinallyNode::FinallyNode(const std::string& name, const NodeConfig& config)
  : ControlNode::ControlNode(name, config)
{
  setRegistrationID("Finally");
}

void FinallyNode::halt()
{
  // halt() also runs from ~Tree(), where a propagating exception terminates, so nothing here throws.
  const bool foreign =
      tls_in_foreign_halt || std::this_thread::get_id() != tick_thread_.load();
  {
    const ForeignHaltScope scope(foreign);
    if(status() == NodeStatus::RUNNING && children_nodes_.size() == 2)
    {
      if(!in_cleanup_)
      {
        haltChildNoThrow(0);
      }
      finishCleanupDuringHalt(foreign);
    }
    for(size_t i = 0; i < children_nodes_.size(); i++)
    {
      haltChildNoThrow(i);
    }
  }
  tick_thread_.store(std::thread::id{});
  in_cleanup_ = false;
  main_started_ = false;
  main_status_ = NodeStatus::IDLE;
  main_exception_ = nullptr;
  resetStatus();
}

void FinallyNode::finishCleanupDuringHalt(bool foreign_thread)
{
  unsigned timeout_msec = kDefaultHaltTimeoutMsec;
  if(auto input = getInput<unsigned>("halt_timeout_msec"))
  {
    timeout_msec = input.value();
  }
  else
  {
    std::cerr << "[" << name() << "]: cannot read halt_timeout_msec (" << input.error()
              << "), using " << kDefaultHaltTimeoutMsec << " ms" << std::endl;
  }
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_msec);
  try
  {
    NodeStatus cleanup_status = children_nodes_[1]->executeTick();
    // Cleanup gets one tick. Keep ticking only if halt_timeout_msec > 0 and this is the thread
    // that ticks the tree, so a halt from a timer or service thread does not block its caller.
    if(timeout_msec > 0 && foreign_thread && cleanup_status == NodeStatus::RUNNING)
    {
      std::cerr << "[" << name()
                << "]: halted from a thread that does not tick this node, cleanup gets a "
                   "single tick instead of halt_timeout_msec"
                << std::endl;
    }
    const bool keep_ticking = timeout_msec > 0 && !foreign_thread;
    while(keep_ticking && cleanup_status == NodeStatus::RUNNING)
    {
      // Start no tick after the deadline, so a tick that blocks cannot extend the wait.
      const auto next_tick = std::chrono::steady_clock::now() + kHaltTickPeriod;
      if(next_tick > deadline)
      {
        std::cerr << "[" << name()
                  << "]: cleanup did not finish within halt_timeout_msec ("
                  << timeout_msec << " ms), halting it unfinished" << std::endl;
        return;
      }
      std::this_thread::sleep_until(next_tick);
      cleanup_status = children_nodes_[1]->executeTick();
    }
    if(cleanup_status == NodeStatus::FAILURE)
    {
      std::cerr << "[" << name() << "]: cleanup returned FAILURE during halt"
                << std::endl;
    }
  }
  catch(...)
  {
    printException(name(), "cleanup threw during halt", std::current_exception());
  }
}

void FinallyNode::haltChildNoThrow(size_t i) noexcept
{
  try
  {
    haltChild(i);
  }
  catch(...)
  {
    printException(name(), "a child threw while being halted", std::current_exception());
  }
}

NodeStatus FinallyNode::tick()
{
  if(children_nodes_.size() != 2)
  {
    throw LogicError("[", name(), "]: Finally requires exactly 2 children");
  }

  if(!isStatusActive(status()))
  {
    in_cleanup_ = false;
    main_started_ = false;
    main_exception_ = nullptr;
  }

  setStatus(NodeStatus::RUNNING);
  tick_thread_.store(std::this_thread::get_id());

  if(!in_cleanup_)
  {
    try
    {
      main_status_ = children_nodes_[0]->executeTick();
    }
    catch(...)
    {
      main_exception_ = std::current_exception();
      haltChildNoThrow(0);
      main_status_ = NodeStatus::FAILURE;
    }

    if(main_status_ == NodeStatus::RUNNING)
    {
      main_started_ = true;
      return NodeStatus::RUNNING;
    }
    if(main_status_ == NodeStatus::IDLE)
    {
      throw LogicError("[", name(), "]: A child should not return IDLE");
    }
    if(main_status_ == NodeStatus::SKIPPED && !main_started_)
    {
      // Main did nothing, so there is nothing to clean up. executeTick() keeps our RUNNING
      // status on SKIPPED, and halt() would run cleanup.
      resetChildren();
      resetStatus();
      return NodeStatus::SKIPPED;
    }
    in_cleanup_ = true;
    main_started_ = false;
  }

  NodeStatus cleanup_status = NodeStatus::IDLE;
  try
  {
    cleanup_status = children_nodes_[1]->executeTick();
  }
  catch(...)
  {
    // As in a finally block, the exception replaces main's outcome and ends this run, so a later
    // halt, such as the one ~Tree() runs, does not retry cleanup.
    // Main finished but was not reset yet. haltChildNoThrow() resets both without throwing, so the
    // cleanup exception is the one that propagates.
    for(size_t i = 0; i < children_nodes_.size(); i++)
    {
      haltChildNoThrow(i);
    }
    in_cleanup_ = false;
    main_exception_ = nullptr;
    resetStatus();
    throw;
  }
  if(cleanup_status == NodeStatus::RUNNING)
  {
    return NodeStatus::RUNNING;
  }

  resetChildren();
  in_cleanup_ = false;
  if(main_exception_)
  {
    // executeTick() keeps our RUNNING status when tick() throws, and halt() would rerun cleanup.
    resetStatus();
    std::rethrow_exception(std::exchange(main_exception_, nullptr));
  }
  if(cleanup_status == NodeStatus::FAILURE)
  {
    return NodeStatus::FAILURE;
  }
  if(main_status_ == NodeStatus::SKIPPED)
  {
    // Main was skipped after it started, e.g. by _while. executeTick() keeps our RUNNING status
    // on SKIPPED, and halt() would rerun cleanup.
    resetStatus();
  }
  return main_status_;
}

}  // namespace BT
