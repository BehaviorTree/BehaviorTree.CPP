#ifndef BEHAVIORTREECORE_TREENODE_H
#define BEHAVIORTREECORE_TREENODE_H


#ifdef DEBUG
  // #define DEBUG_STDERR(x) (std::cerr << (x))
#define DEBUG_STDOUT(str) do { std::cout << str << std::endl; } while( false )

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

    enum NodeType {ACTION_NODE, CONDITION_NODE, CONTROL_NODE, DECORATOR_NODE};

    // Enumerates the states every node can be in after execution during a particular
    // time step:
    // - "Success" indicates that the node has completed running during this time step;
    // - "Failure" indicates that the node has determined it will not be able to complete
    //   its task;
    // - "Running" indicates that the node has successfully moved forward during this
    //   time step, but the task is not yet complete;
    // - "Idle" indicates that the node hasn't run yet.
    // - "Halted" indicates that the node has been halted by its father.
    enum ReturnStatus {RUNNING, SUCCESS, FAILURE, IDLE, HALTED, EXIT};

    // Enumerates the options for when a parallel node is considered to have failed:
    // - "FAIL_ON_ONE" indicates that the node will return failure as soon as one of
    //   its children fails;
    // - "FAIL_ON_ALL" indicates that all of the node's children must fail before it
    //   returns failure.
    enum FailurePolicy {FAIL_ON_ONE, FAIL_ON_ALL};
    enum ResetPolity   {ON_SUCCESS_OR_FAILURE,ON_SUCCESS, ON_FAILURE};

    // Enumerates the options for when a parallel node is considered to have succeeded:
    // - "SUCCEED_ON_ONE" indicates that the node will return success as soon as one
    //   of its children succeeds;
    // - "BT::SUCCEED_ON_ALL" indicates that all of the node's children must succeed before
    //   it returns success.
    enum SuccessPolicy {SUCCEED_ON_ONE, SUCCEED_ON_ALL};

    // If "BT::FAIL_ON_ONE" and "BT::SUCCEED_ON_ONE" are both active and are both trigerred in the
    // same time step, failure will take precedence.

    // Abstract base class for Behavior Tree Nodes
    class TreeNode
    {
    private:
        // Node name
        std::string name_;

    protected:
        // The node state that must be treated in a thread-safe way
        bool is_state_updated_;
        ReturnStatus status_;

        mutable std::mutex state_mutex_;
        std::condition_variable state_condition_variable_;

    public:

        // Node semaphore to simulate the tick
        // (and to synchronize fathers and children)
        TickEngine tick_engine;

        // The constructor and the distructor
        TreeNode(std::string name);
        virtual ~TreeNode();

        // The method that is going to be executed when the node receive a tick
        virtual BT::ReturnStatus Tick() = 0;

        // The method used to interrupt the execution of the node
        virtual void Halt() = 0;

        bool IsHalted() const;

        ReturnStatus Status() const;
        void SetStatus(ReturnStatus new_status);

        const std::string& Name() const;
        void SetName(const std::string& new_name);

        virtual NodeType Type() const = 0;

    };
}

#endif
