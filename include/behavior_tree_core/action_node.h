/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018 Davide Faconti -  All Rights Reserved
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
#include "leaf_node.h"

namespace BT
{
class ActionNodeBase : public LeafNode
{
  public:
    // Constructor
    ActionNodeBase(std::string name);
    ~ActionNodeBase() = default;

    virtual NodeType type() const override final
    {
        return ACTION_NODE;
    }
};

/**
 * @brief The SimpleActionNode provides a easy to use ActionNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus funtionName(void)
 *
 * This avoids the hassle of inheriting from a ActionNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method too.
 * For the time being, this class of Actions can not carry use the BT::NodeParameters
 * This may change in the future.
 */
class SimpleActionNode : public ActionNodeBase
{
  public:
    typedef std::function<NodeStatus()> TickFunctor;

    // Constructor: you must provide the funtion to call when tick() is invoked
    SimpleActionNode(std::string name, TickFunctor tick_functor);

    ~SimpleActionNode() = default;

    virtual NodeStatus tick() override;

    virtual void halt() override
    {
        // not supported
    }

  protected:
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


class ActionNode : public ActionNodeBase
{
  public:
    // Constructor
    ActionNode(std::string name);
    virtual ~ActionNode();

    // The method that is going to be executed by the thread
    void waitForTick();

    // This method triggers the TickEngine. Do NOT remove the "final" keyword.
    virtual NodeStatus executeTick() override final;

    // This method MUST to be overriden by the user.
    virtual NodeStatus tick() override
    {
        return BT::HALTED;
    }

    // This method MUST to be overriden by the user.
    virtual void halt() override
    {
        setStatus(BT::HALTED);
    }

    void stopAndJoinThread();

  protected:
    // The thread that will execute the node
    std::thread thread_;

    // Node semaphore to simulate the tick
    // (and to synchronize fathers and children)
    TickEngine tick_engine_;

    std::atomic<bool> loop_;
};

}   //end namespace

#endif
