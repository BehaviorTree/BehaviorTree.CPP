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

#ifndef BEHAVIORTREECORE_ACTIONNODE_H
#define BEHAVIORTREECORE_ACTIONNODE_H

#include <atomic>
#include <thread>
#include "leaf_node.h"

namespace BT
{
/** IMPORTANT: to avoid unexpected behaviors when Sequence (not SequenceStar) is used
 * an Action that returned SUCCESS or FAILURE will not be ticked again unless
 * setStatus(IDLE) is called first (reset the Action).
 *
 * Usually the parent node takes care of this for you.
 */


class ActionNodeBase : public LeafNode
{
  public:

    ActionNodeBase(const std::string& name, const NodeConfiguration& config);
    ~ActionNodeBase() override = default;

    virtual NodeStatus executeTick() override;

    virtual NodeType type() const override final
    {
        return NodeType::ACTION;
    }
};

/**
 * @brief The SyncActionNode is an helper derived class that
 * explicitly forbids the status RUNNING and doesn't require
 * an implementation of halt()
 */
class SyncActionNode : public ActionNodeBase
{
  public:

    SyncActionNode(const std::string& name, const NodeConfiguration& config);
    ~SyncActionNode() override = default;

    virtual NodeStatus executeTick() override;

    virtual void halt() override final // don't need to override this
    {
        setStatus(NodeStatus::IDLE);
    }
};

/**
 * @brief The SimpleActionNode provides an easy to use ActionNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(void)
 *
 * This avoids the hassle of inheriting from a ActionNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 * SimpleActionNode is executed synchronously and does not support halting.
 * NodeParameters aren't supported.
 */
class SimpleActionNode : public ActionNodeBase
{
  public:
    typedef std::function<NodeStatus(TreeNode&)> TickFunctor;

    // Constructor: you must provide the function to call when tick() is invoked
    SimpleActionNode(const std::string& name, TickFunctor tick_functor,
                     const NodeConfiguration& config);

    ~SimpleActionNode() override = default;

    virtual void halt() override
    {
        // not supported
    }

  protected:
    virtual NodeStatus tick() override;

    TickFunctor tick_functor_;
};

/**
 * @brief The AsyncActionNode a different thread where the action will be
 * executed.
 *
 * The user must implement the method asyncTick() instead of tick() and
 * the method halt() as usual.
 *
 * Remember, though, that the method asyncTick() must update the state to either
 * RUNNING, SUCCESS or FAILURE, otherwise the execution of the Behavior Tree is blocked!
 *
 */
class AsyncActionNode : public ActionNodeBase
{
  public:

    AsyncActionNode(const std::string& name, const NodeConfiguration& config);
    virtual ~AsyncActionNode() override;

    // This method triggers the TickEngine. Do NOT remove the "final" keyword.
    virtual NodeStatus executeTick() override final;

    void stopAndJoinThread();

  private:

    // The method that is going to be executed by the thread
    void waitForTick();

    void waitStart();

    void notifyStart();

    std::thread thread_;

    std::atomic<bool> loop_;

    bool start_action_;

    std::mutex mutex_;

    std::condition_variable condition_variable_;
};

// Why is the name "ActionNode" deprecated?
//
// ActionNode was renamed "AsyncActionNode" because it's implementation, i.e. one thread
// per action, is too wastefull in terms of resources.
// The name ActionNode seems to imply that it is the default Node to use for Actions.
// But, in my opinion, the user should think twice if using it and carefully consider the cost of abstraction.
// For this reason, AsyncActionNode is a much better name.


// The right class to use for synchronous Actions is SyncActionBase
[[deprecated]]
typedef AsyncActionNode ActionNode;

/**
 * @brief The CoroActionNode class is an ideal candidate for asynchronous actions
 * which need to communicate with a service provider using an asynch request/reply interface
 * (being a notable example ActionLib in ROS, MoveIt clients or move_base clients).
 *
 * It is up to the user to decide when to suspend execution of the behaviorTree invoking
 * the method setStatusRunningAndYield().
 */
class CoroActionNode : public ActionNodeBase
{
  public:
    // Constructor
    CoroActionNode(const std::string& name, const NodeConfiguration& config);
    virtual ~CoroActionNode() override;

    /** When you want to return RUNNING and temporary "pause"
    *   the Action, use this method.
    * */
    void setStatusRunningAndYield();

    // This method triggers the TickEngine. Do NOT remove the "final" keyword.
    virtual NodeStatus executeTick() override final;

    /** You may want to override this method. But still, call remember to call this
    * implementation too.
    *
    * Example:
    *
    *     void MyAction::halt()
    *     {
    *         // do your stuff here
    *         CoroActionNode::halt();
    *     }
    */
    void halt() override;

  protected:

    struct Pimpl; // The Pimpl idiom
    std::unique_ptr<Pimpl> _p;

};


}   //end namespace

#endif
