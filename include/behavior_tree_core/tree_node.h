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

#define DEBUG

#ifdef DEBUG
// #define DEBUG_STDERR(x) (std::cerr << (x))
#define DEBUG_STDOUT(str)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        std::cout << str << std::endl;                                                                                 \
    } while (false)

#else
#define DEBUG_STDOUT(str)
#endif

#include <iostream>
//#include <unistd.h>

#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "behavior_tree_core/tick_engine.h"
#include "behavior_tree_core/exceptions.h"

namespace BT
{
// Enumerates the possible types of a node, for drawinf we have do discriminate whoich control node it is:

enum NodeType
{
    ACTION_NODE,
    CONDITION_NODE,
    CONTROL_NODE,
    DECORATOR_NODE
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
enum NodeStatus
{
    RUNNING,
    SUCCESS,
    FAILURE,
    IDLE,
    HALTED,
    EXIT
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

// If "BT::FAIL_ON_ONE" and "BT::SUCCEED_ON_ONE" are both active and are both trigerred in the
// same time step, failure will take precedence.

// Abstract base class for Behavior Tree Nodes
class TreeNode
{
  private:
    // Node name
    std::string name_;
    NodeStatus status_;

  protected:
    // The node state that must be treated in a thread-safe way
    bool is_state_updated_;

    mutable std::mutex state_mutex_;
    std::condition_variable state_condition_variable_;

  public:
    // Node semaphore to simulate the tick
    // (and to synchronize fathers and children)
    TickEngine tick_engine;

    // The constructor and the distructor
    TreeNode(std::string name);
    virtual ~TreeNode() = default;

    // The method that is going to be executed when the node receive a tick
    virtual BT::NodeStatus tick() = 0;

    // The method used to interrupt the execution of the node
    virtual void halt() = 0;

    bool isHalted() const;

    NodeStatus status() const;
    void setStatus(NodeStatus new_status);

    const std::string& name() const;
    void setName(const std::string& new_name);

    BT::NodeStatus waitValidStatus();

    virtual NodeType type() const = 0;
};
}

#endif
