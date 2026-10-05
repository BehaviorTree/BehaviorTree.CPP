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

void printException(std::string_view node_name, const char* context,
                    const std::exception_ptr& exception)
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
  if(status() == NodeStatus::RUNNING && children_nodes_.size() == 2)
  {
    if(!in_cleanup_)
    {
      haltChildNoThrow(0);
    }
    finishCleanupDuringHalt();
  }
  for(size_t i = 0; i < children_nodes_.size(); i++)
  {
    haltChildNoThrow(i);
  }
  in_cleanup_ = false;
  main_status_ = NodeStatus::IDLE;
  main_exception_ = nullptr;
  resetStatus();
}

void FinallyNode::finishCleanupDuringHalt()
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
    // Cleanup is commonly asynchronous, so keep ticking it until it finishes rather than
    // halting it after its first tick.
    NodeStatus cleanup_status = children_nodes_[1]->executeTick();
    while(cleanup_status == NodeStatus::RUNNING)
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

void FinallyNode::haltChildNoThrow(size_t i)
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
    main_exception_ = nullptr;
  }

  setStatus(NodeStatus::RUNNING);

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
      return NodeStatus::RUNNING;
    }
    if(main_status_ == NodeStatus::IDLE)
    {
      throw LogicError("[", name(), "]: A child should not return IDLE");
    }
    in_cleanup_ = true;
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
    // executeTick() keeps our RUNNING status on SKIPPED, and halt() would rerun cleanup.
    resetStatus();
  }
  return main_status_;
}

}  // namespace BT
