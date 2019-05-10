/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/action_node.h"
#include "coroutine/coroutine.h"

namespace BT
{
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
  : ActionNodeBase(name, config),
  keep_thread_alive_(true),
  start_action_(false)
{
    thread_ = std::thread(&AsyncActionNode::asyncThreadLoop, this);
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
    notifyStart();
    if (thread_.joinable())
    {
        thread_.join();
    }
}

//-------------------------------------
struct CoroActionNode::Pimpl
{
    coroutine::routine_t coro;
    std::atomic<bool> pending_destroy;

};


CoroActionNode::CoroActionNode(const std::string &name,
                               const NodeConfiguration& config):
  ActionNodeBase (name, config),
  _p(new  Pimpl)
{
    _p->coro = 0;
    _p->pending_destroy = false;
}

CoroActionNode::~CoroActionNode()
{
    if( _p->coro != 0 )
    {
        coroutine::destroy(_p->coro);
    }
}

void CoroActionNode::setStatusRunningAndYield()
{
    setStatus( NodeStatus::RUNNING );
    coroutine::yield();
}

NodeStatus CoroActionNode::executeTick()
{
    if( _p->pending_destroy && _p->coro != 0 )
    {
        coroutine::destroy(_p->coro);
        _p->coro = 0;
        _p->pending_destroy = false;
    }

    if ( _p->coro == 0)
    {
        _p->coro = coroutine::create( [this]()
        {
            setStatus(tick());
        } );
    }

    if( _p->coro != 0 )
    {
        if( _p->pending_destroy ||
            coroutine::resume(_p->coro) == coroutine::ResumeResult::FINISHED )
        {
            coroutine::destroy(_p->coro);
            _p->coro = 0;
            _p->pending_destroy = false;
        }
    }
    return status();
}

void CoroActionNode::halt()
{
    _p->pending_destroy = true;
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



}
