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

#ifndef BEHAVIORTREECORE_TREENODE_H
#define BEHAVIORTREECORE_TREENODE_H

#include <iostream>
#include <string>
#include <map>
#include <set>

#include "behavior_tree_core/tick_engine.h"
#include "behavior_tree_core/exceptions.h"
#include "behavior_tree_core/signal.h"

namespace BT
{
// Enumerates the possible types of a node, for drawinf we have do discriminate whoich control node it is:

enum class NodeType
{
    UNDEFINED = 0,
    ACTION,
    CONDITION,
    CONTROL,
    DECORATOR,
    SUBTREE
};

// Enumerates the states every node can be in after execution during a particular
// time step:
// - "Success" indicates that the node has completed running during this time step;
// - "Failure" indicates that the node has determined it will not be able to complete
//   its task;
// - "Running" indicates that the node has successfully moved forward during this
//   time step, but the task is not yet complete;
// - "Idle" indicates that the node hasn't run yet.
// - "Halted" indicates that the node has been halted by its father.
enum class NodeStatus
{
    IDLE = 0,
    RUNNING,
    SUCCESS,
    FAILURE
};


// Enumerates the options for when a parallel node is considered to have failed:
// - "FAIL_ON_ONE" indicates that the node will return failure as soon as one of
//   its children fails;
// - "FAIL_ON_ALL" indicates that all of the node's children must fail before it
//   returns failure.
enum FailurePolicy
{
    FAIL_ON_ONE,
    FAIL_ON_ALL
};
enum ResetPolicy
{
    ON_SUCCESS_OR_FAILURE,
    ON_SUCCESS,
    ON_FAILURE
};

// Enumerates the options for when a parallel node is considered to have succeeded:
// - "SUCCEED_ON_ONE" indicates that the node will return success as soon as one
//   of its children succeeds;
// - "BT::SUCCEED_ON_ALL" indicates that all of the node's children must succeed before
//   it returns success.
enum SuccessPolicy
{
    SUCCEED_ON_ONE,
    SUCCEED_ON_ALL
};

// If "BT::FAIL_vON_ONE" and "BT::SUCCEED_ON_ONE" are both active and are both trigerred in the
// same time step, failure will take precedence.

// We call Parameters the set of Key/Values that can be read from file and are
// used to parametrize an object. It is up to the user's code to parse the string.
typedef std::map<std::string, std::string> NodeParameters;

// Abstract base class for Behavior Tree Nodes
class TreeNode
{
  private:
    // Node name
    std::string name_;
    NodeStatus status_;
    std::condition_variable state_condition_variable_;
    mutable std::mutex state_mutex_;

  protected:

    // Method to be implemented by the user
    virtual BT::NodeStatus tick() = 0;

  public:
    // The constructor and the distructor
    TreeNode(std::string name);
    virtual ~TreeNode() = default;

    // The method that is going to be executed when the node receive a tick
    virtual BT::NodeStatus executeTick();

    // The method used to interrupt the execution of the node
    virtual void halt() = 0;

    bool isHalted() const;

    NodeStatus status() const;
    void setStatus(NodeStatus new_status);

    const std::string& name() const;
    void setName(const std::string& new_name);

    BT::NodeStatus waitValidStatus();

    virtual NodeType type() const = 0;

    using StatusChangeSignal = Signal<const TreeNode&, NodeStatus,NodeStatus>;
    using StatusChangeSubscriber = StatusChangeSignal::Subscriber;
    using StatusChangeCallback   = StatusChangeSignal::CallableFunction;

    /**
     * @brief subscribeToStatusChange is used to attach a callback to a status change.
     * AS soon as StatusChangeSubscriber goes out of scope (it is a shared_ptr) the callback
     * is unsubscribed
     *
     * @param callback. Must have signature void funcname(NodeStatus prev_status, NodeStatus new_status)
     *
     * @return the subscriber.
     */
     StatusChangeSubscriber subscribeToStatusChange(StatusChangeCallback callback);

     // get an unique identifier of this instance of treeNode
     uint16_t UID() const;

private:

  StatusChangeSignal state_change_signal_;

  const uint16_t _uid;

};

typedef std::shared_ptr<TreeNode> TreeNodePtr;

// The term "Builder" refers to the Builder Pattern (https://en.wikipedia.org/wiki/Builder_pattern)
typedef std::function<std::unique_ptr<TreeNode>(const std::string&, const NodeParameters&)> NodeBuilder;
}

#endif
