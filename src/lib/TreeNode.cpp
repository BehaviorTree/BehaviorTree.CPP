#include <TreeNode.h>

using namespace BT;

TreeNode::TreeNode(std::string Name) : Semaphore(0)
{
    // Initialization
    this->Name = Name;
    StateUpdated = false;
    State = Idle;
}

TreeNode::~TreeNode() {}

NodeState TreeNode::GetNodeState()
{
    NodeState ReadState;
    // Lock acquistion
    boost::unique_lock<boost::mutex> UniqueLock(StateMutex);

    // Wait until the state is updated by the node thread
    while(StateUpdated == false)
        StateConditionVariable.wait(UniqueLock);

    // Reset the StateUpdated flag
    StateUpdated = false;

    // State save
    ReadState = State;

    // Releasing the node thread;
    StateConditionVariable.notify_all();

    // Take the state and unlock the mutex
    return ReadState;
}

void TreeNode::SetNodeState(NodeState StateToBeSet)
{

    if(StateToBeSet != Idle)
    {
        SetColorState(StateToBeSet);
    }

    // Lock acquistion
    boost::unique_lock<boost::mutex> UniqueLock(StateMutex);

    // State update
    State = StateToBeSet;
    StateUpdated = true;

    // Notification and unlock of the mutex
    StateConditionVariable.notify_all();

    // Waiting for the father to read the state
    StateConditionVariable.wait(UniqueLock);
}

NodeState TreeNode::ReadState()
{
    // Lock acquistion
    boost::lock_guard<boost::mutex> LockGuard(StateMutex);

    return State;
}


NodeState TreeNode::ReadColorState()
{
    // Lock acquistion

    return ColorState;
}

void TreeNode::SetColorState(NodeState ColorStateToBeSet)
{
    // Lock acquistion

    // State update
    ColorState = ColorStateToBeSet;
}




