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

// IMPORTANT: Actions which returned SUCCESS or FAILURE will not be ticked
// again unless setStatus(IDLE) is called first.
// Keep this in mind when writing your custom Control and Decorator nodes.


/**
 * @brief The ActionNodeBase is the base class to use to create any kind of action.
 * A particular derived class is free to override executeTick() as needed.
 *
 */
class ActionNodeBase : public LeafNode
{
  public:

    ActionNodeBase(const std::string& name, const NodeConfiguration& config);
    ~ActionNodeBase() override = default;

    virtual NodeType type() const override final
    {
        return NodeType::ACTION;
    }
};

/**
 * @brief The SyncActionNode is an ActionNode that
 * explicitly prevents the status RUNNING and doesn't require
 * an implementation of halt().
 */
class SyncActionNode : public ActionNodeBase
{
  public:

    SyncActionNode(const std::string& name, const NodeConfiguration& config);
    ~SyncActionNode() override = default;

    /// throws if the derived class return RUNNING.
    virtual NodeStatus executeTick() override;

    /// You don't need to override this
    virtual void halt() override final
    {
        setStatus(NodeStatus::IDLE);
    }
};

/**
 * @brief The SimpleActionNode provides an easy to use SyncActionNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(TreeNode&)
 *
 * This avoids the hassle of inheriting from a ActionNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 * SimpleActionNode is executed synchronously and does not support halting.
 * NodeParameters aren't supported.
 */
class SimpleActionNode : public SyncActionNode
{
  public:
    typedef std::function<NodeStatus(TreeNode&)> TickFunctor;

    // You must provide the function to call when tick() is invoked
    SimpleActionNode(const std::string& name, TickFunctor tick_functor,
                     const NodeConfiguration& config);

    ~SimpleActionNode() override = default;

  protected:
    virtual NodeStatus tick() override final;

    TickFunctor tick_functor_;
};

/**
 * @brief The AsyncActionNode uses a different thread where the action will be
 * executed.
 *
 * The user must implement the methods tick() and halt().
 *
 * WARNING: this should probably be deprecated. It is too easy to use incorrectly
 * and there is not a good way to halt it in a thread safe way.
 *
 * Use it at your own risk.
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

    // The method that will be executed by the thread
    void asyncThreadLoop();

    void waitStart();

    void notifyStart();

    std::atomic<bool> keep_thread_alive_;

    bool start_action_;

    std::mutex start_mutex_;

    std::condition_variable start_signal_;

    std::exception_ptr exptr_;

    std::thread thread_;
};

/**
 * @brief The ActionNode is the goto option for,
 * but it is actually much easier to use correctly.
 *
 * It is particularly useful when your code contains a request-reply pattern,
 * i.e. when the actions sends an asychronous request, then checks periodically
 * if the reply has been received and, eventually, analyze the reply to determine
 * if the result is SUCCESS or FAILURE.
 *
 * -) an IDLE action will call onStart()
 *
 * -) A RUNNING action will call onRunning()
 *
 * -) if halted, method onHalted()
 */
class StatefulActionNode : public ActionNodeBase
{
  public:
      StatefulActionNode(const std::string& name, const NodeConfiguration& config):
        ActionNodeBase(name,config)
      {}

      // do not override this method
      NodeStatus tick() override final;
      // do not override this method
      void halt() override final;

      /// method to be called at the beginning.
      /// If it returns RUNNING, this becomes an asychronous node.
      virtual NodeStatus onStart() = 0;

      /// method invoked by a RUNNING action.
      virtual NodeStatus onRunning() = 0;

      /// when the method halt() is called by a parent node, this method
      /// is invoked to do the cleanup of a RUNNING action.
      virtual void onHalted() = 0;
};


#ifndef BT_NO_COROUTINES

/**
 * @brief The CoroActionNode class is an ideal candidate for asynchronous actions
 * which need to communicate with an external service using an asynch request/reply interface
 * (being notable examples ActionLib in ROS, MoveIt clients or move_base clients).
 *
 * It is up to the user to decide when to suspend execution of the Action and resume
 * the parent node, invoking the method setStatusRunningAndYield().
 */
class CoroActionNode : public ActionNodeBase
{
  public:

    CoroActionNode(const std::string& name, const NodeConfiguration& config);
    virtual ~CoroActionNode() override;

    /// Use this method to return RUNNING and temporary "pause" the Action.
    void setStatusRunningAndYield();

    // This method triggers the TickEngine. Do NOT remove the "final" keyword.
    virtual NodeStatus executeTick() override final;

    /** You may want to override this method. But still, remember to call this
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
#endif

}   //end namespace

#endif
