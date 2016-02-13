#ifndef TREENODE_H
#define TREENODE_H

#include <iostream>
#include <unistd.h>

#include <string>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <NodeSemaphore.h>
#include <Exceptions.h>

namespace BT
{
    // Enumerates the possible types of a node:
    // - "Parallel" indicates that this node and its predecessor are children
    //   (not necessary direct ones!) of a parallel node;
    // - "Selector" indicates that the two node are children of a selector node;
    // - "Sequence" same as above, but with a sequence node as father;
    enum NodeType {Action, Condition, Control};
    enum ColorNodeType {PARALLEL, SELECTOR, SEQUENCE, SEQUENCESTAR, SELECTORSTAR, ACTION, CONDITION,DECORATOR};
    // Enumerates the states every node can be in after execution during a particular
    // time step:
    // - "Success" indicates that the node has completed running during this time step;
    // - "Failure" indicates that the node has determined it will not be able to complete
    //   its task;
    // - "Running" indicates that the node has successfully moved forward during this
    //   time step, but the task is not yet complete;
    // - "Idle" indicates that the node hasn't run yet.
    // - "Halted" indicates that the node has been halted by its father.
    enum NodeState {Success, Failure, Running, Idle, Halted, Exit};

    // Enumerates the options for when a parallel node is considered to have failed:
    // - "FailOnOne" indicates that the node will return failure as soon as one of
    //   its children fails;
    // - "FailOnAll" indicates that all of the node's children must fail before it
    //   returns failure.
    enum FailurePolicy {FailOnOne, FailOnAll};

    // Enumerates the options for when a parallel node is considered to have succeeded:
    // - "SucceedOnOne" indicates that the node will return success as soon as one
    //   of its children succeeds;
    // - "SucceedOnAll" indicates that all of the node's children must succeed before
    //   it returns success.
    enum SuccessPolicy {SucceedOnOne, SucceedOnAll};

    // If "FailOnOne" and "SucceedOnOne" are both active and are both trigerred in the
    // same time step, failure will take precedence.

    // Abstract base class for Behavior Tree Nodes
    class TreeNode
    {
    protected:
        // The node state that must be treated in a thread-safe way
        bool StateUpdated;
        NodeState State;
        NodeState ColorState;
        boost::mutex StateMutex;
        boost::mutex ColorStateMutex;
        boost::condition_variable StateConditionVariable;
    public:
        // Node name
        std::string Name;

        // Node type
        NodeType Type;

        // The thread that will execute the node
        boost::thread Thread;

        // Node semaphore to simulate the tick
        // (and to synchronize fathers and children)
        NodeSemaphore Semaphore;

        // The constructor and the distructor
        TreeNode(std::string Name);
        ~TreeNode();

        // The method that is going to be executed by the thread
        virtual void Exec() = 0;

        // The method used to interrupt the execution of the node
        virtual bool Halt() = 0;

        // The method that retrive the state of the node
        // (conditional waiting and mutual access)
        NodeState GetNodeState();
        void SetNodeState(NodeState StateToBeSet);
        void SetColorState(NodeState ColorStateToBeSet);

        // Methods used to access the node state without the
        // conditional waiting (only mutual access)
        NodeState ReadState();
        NodeState ReadColorState();
        virtual int GetType() = 0;
        virtual bool WriteState(NodeState StateToBeSet) = 0;
        virtual void ResetColorState() = 0;
        virtual int GetDepth() = 0;

    };
}

#endif
