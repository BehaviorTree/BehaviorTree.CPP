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

#include "behaviortree_cpp_v3/action_node.h"

using namespace BT;

ActionNodeBase::ActionNodeBase(const std::string& name, const NodeConfiguration& config) :
  LeafNode::LeafNode(name, config)
{}

//-------------------------------------------------------

SimpleActionNode::SimpleActionNode(const std::string& name,
                                   SimpleActionNode::TickFunctor tick_functor,
                                   const NodeConfiguration& config) :
  SyncActionNode(name, config), tick_functor_(std::move(tick_functor))
{}

NodeStatus SimpleActionNode::tick()
{
  NodeStatus prev_status = status();

  if (prev_status == NodeStatus::IDLE)
  {
    setStatus(NodeStatus::RUNNING);
    prev_status = NodeStatus::RUNNING;
  }

  NodeStatus status = tick_functor_(*this);
  if (status != prev_status)
  {
    setStatus(status);
  }
  return status;
}

//-------------------------------------------------------

SyncActionNode::SyncActionNode(const std::string& name, const NodeConfiguration& config) :
  ActionNodeBase(name, config)
{}

NodeStatus SyncActionNode::executeTick()
{
  auto stat = ActionNodeBase::executeTick();
  if (stat == NodeStatus::RUNNING)
  {
    throw LogicError("SyncActionNode MUST never return RUNNING");
  }
  return stat;
}

//-------------------------------------
#ifndef BT_NO_COROUTINES

#ifdef BT_BOOST_COROUTINE2
#include <boost/coroutine2/all.hpp>
using namespace boost::coroutines2;
#endif

#ifdef BT_BOOST_COROUTINE
#include <boost/coroutine/all.hpp>
using namespace boost::coroutines;
#endif

struct CoroActionNode::Pimpl
{
  std::unique_ptr<coroutine<void>::pull_type> coro;
  std::function<void(coroutine<void>::push_type& yield)> func;
  coroutine<void>::push_type* yield_ptr;
};

CoroActionNode::CoroActionNode(const std::string& name, const NodeConfiguration& config) :
  ActionNodeBase(name, config), _p(new Pimpl)
{
  _p->func = [this](coroutine<void>::push_type& yield) {
    _p->yield_ptr = &yield;
    setStatus(tick());
  };
}

CoroActionNode::~CoroActionNode()
{}

void CoroActionNode::setStatusRunningAndYield()
{
  setStatus(NodeStatus::RUNNING);
  (*_p->yield_ptr)();
}

NodeStatus CoroActionNode::executeTick()
{
  if (!(_p->coro) || !(*_p->coro))
  {
    _p->coro.reset(new coroutine<void>::pull_type(_p->func));
    return status();
  }

  if (status() == NodeStatus::RUNNING && (bool)_p->coro)
  {
    (*_p->coro)();
  }

  return status();
}

void CoroActionNode::halt()
{
  _p->coro.reset();
}
#endif

NodeStatus StatefulActionNode::tick()
{
  const NodeStatus initial_status = status();

  if (initial_status == NodeStatus::IDLE)
  {
    NodeStatus new_status = onStart();
    if (new_status == NodeStatus::IDLE)
    {
      throw std::logic_error("StatefulActionNode::onStart() must not return IDLE");
    }
    return new_status;
  }
  //------------------------------------------
  if (initial_status == NodeStatus::RUNNING)
  {
    NodeStatus new_status = onRunning();
    if (new_status == NodeStatus::IDLE)
    {
      throw std::logic_error("StatefulActionNode::onRunning() must not return "
                             "IDLE");
    }
    return new_status;
  }
  //------------------------------------------
  return initial_status;
}

void StatefulActionNode::halt()
{
  if (status() == NodeStatus::RUNNING)
  {
    onHalted();
  }
}

NodeStatus BT::AsyncActionNode::executeTick()
{
  using lock_type = std::unique_lock<std::mutex>;
  //send signal to other thread.
  // The other thread is in charge for changing the status
  if (status() == NodeStatus::IDLE)
  {
    setStatus(NodeStatus::RUNNING);
    halt_requested_ = false;
    thread_handle_ = std::async(std::launch::async, [this]() {
      try
      {
        auto status = tick();
        if (!isHaltRequested())
        {
          setStatus(status);
        }
      }
      catch (std::exception&)
      {
        std::cerr << "\nUncaught exception from the method tick(): ["
                  << registrationName() << "/" << name() << "]\n"
                  << std::endl;
        // Set the exception pointer and the status atomically.
        lock_type l(mutex_);
        exptr_ = std::current_exception();
        setStatus(BT::NodeStatus::IDLE);
      }
      emitStateChanged();
    });
  }

  lock_type l(mutex_);
  if (exptr_)
  {
    // The official interface of std::exception_ptr does not define any move
    // semantics. Thus, we copy and reset exptr_ manually.
    const auto exptr_copy = exptr_;
    exptr_ = nullptr;
    std::rethrow_exception(exptr_copy);
  }
  return status();
}

void AsyncActionNode::halt()
{
  halt_requested_.store(true);

  if (thread_handle_.valid())
  {
    thread_handle_.wait();
  }
  thread_handle_ = {};
}
