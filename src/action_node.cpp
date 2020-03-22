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

ActionNodeBase::ActionNodeBase(const std::string& name, const NodeConfiguration& config)
  : LeafNode::LeafNode(name, config)
{
}


//-------------------------------------------------------

SimpleActionNode::SimpleActionNode(const std::string& name,
                                   SimpleActionNode::TickFunctor tick_functor,
                                   const NodeConfiguration& config)
  : SyncActionNode(name, config), tick_functor_(std::move(tick_functor))
{
}

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

AsyncActionNode::AsyncActionNode(const std::string& name, const NodeConfiguration& config)
  : ActionNodeBase(name, config)
{

}

AsyncActionNode::~AsyncActionNode()
{
    if (thread_.joinable())
    {
        stopAndJoinThread();
    }
}

void AsyncActionNode::waitStart()
{
    std::unique_lock<std::mutex> lock(start_mutex_);
    while (!start_action_)
    {
        start_signal_.wait(lock);
    }
    start_action_ = false;
}

void AsyncActionNode::notifyStart()
{
    std::unique_lock<std::mutex> lock(start_mutex_);
    start_action_ = true;
    start_signal_.notify_all();
}

void AsyncActionNode::asyncThreadLoop()
{
    while (keep_thread_alive_.load())
    {
        waitStart();

        // check keep_thread_alive_ again because the tick_engine_ could be
        // notified from the method stopAndJoinThread
        if (keep_thread_alive_)
        {
            // this will execute the blocking code.
            try {
                setStatus(tick());
            }
            catch (std::exception&)
            {
                std::cerr << "\nUncaught exception from the method tick() of an AsyncActionNode: ["
                          << registrationName() << "/" << name() << "]\n" << std::endl;
                exptr_ = std::current_exception();
                keep_thread_alive_ = false;
            }
        }
    }
}

NodeStatus AsyncActionNode::executeTick()
{
    //send signal to other thread.
    // The other thread is in charge for changing the status
    if (status() == NodeStatus::IDLE)
    {
        if( thread_.joinable() == false) {
            keep_thread_alive_ = true;
            thread_ = std::thread(&AsyncActionNode::asyncThreadLoop, this);
        }
        setStatus( NodeStatus::RUNNING );
        notifyStart();
    }

    if( exptr_ )
    {
        std::rethrow_exception(exptr_);
    }
    return status();
}

void AsyncActionNode::stopAndJoinThread()
{
    keep_thread_alive_.store(false);
    if( status() == NodeStatus::RUNNING )
    {
        halt();
    }
    else{
        // loop in asyncThreadLoop() is blocked at waitStart(). Unblock it.
        notifyStart();
    }

    if (thread_.joinable())
    {
        thread_.join();
    }
}


SyncActionNode::SyncActionNode(const std::string &name, const NodeConfiguration& config):
  ActionNodeBase(name, config)
{}

NodeStatus SyncActionNode::executeTick()
{
  auto stat = ActionNodeBase::executeTick();
  if( stat == NodeStatus::RUNNING)
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
    std::function<void(coroutine<void>::push_type & yield)> func;
    coroutine<void>::push_type * yield_ptr;
};

CoroActionNode::CoroActionNode(const std::string &name,
                               const NodeConfiguration& config):
 ActionNodeBase (name, config), _p( new Pimpl)
{
    _p->func = [this](coroutine<void>::push_type & yield) {
        _p->yield_ptr = &yield;
        setStatus(tick());
    };
}

CoroActionNode::~CoroActionNode()
{
}

void CoroActionNode::setStatusRunningAndYield()
{
    setStatus( NodeStatus::RUNNING );
    (*_p->yield_ptr)();
}

NodeStatus CoroActionNode::executeTick()
{
    if( !(_p->coro) || !(*_p->coro) )
    {
        _p->coro.reset( new coroutine<void>::pull_type(_p->func) );
        return status();
    }

    if( status() == NodeStatus::RUNNING && (bool)_p->coro )
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

  if( initial_status == NodeStatus::IDLE )
  {
    NodeStatus new_status = onStart();
    if( new_status == NodeStatus::IDLE)
    {
      throw std::logic_error("AsyncActionNode2::onStart() must not return IDLE");
    }
    return new_status;
  }
  //------------------------------------------
  if( initial_status == NodeStatus::RUNNING )
  {
    NodeStatus new_status = onRunning();
    if( new_status == NodeStatus::IDLE)
    {
      throw std::logic_error("AsyncActionNode2::onRunning() must not return IDLE");
    }
    return new_status;
  }
  //------------------------------------------
  return initial_status;
}

void StatefulActionNode::halt()
{
  if( status() == NodeStatus::RUNNING)
  {
    onHalted();
  }
  setStatus(NodeStatus::IDLE);
}
