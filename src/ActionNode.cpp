#include <ActionNode.h>

using namespace BT;

ActionNode::ActionNode(std::string Name) : LeafNode::LeafNode(Name)
{
    Type = Action;
}

ActionNode::~ActionNode() {}

bool ActionNode::WriteState(NodeState StateToBeSet)
{

    if(StateToBeSet != Idle)
    {
        SetColorState(StateToBeSet);
    }
    // Lock acquistion
    boost::lock_guard<boost::mutex> LockGuard(StateMutex);

    // Check for spourios "Halted"
    if (State == Halted && StateToBeSet != Idle && StateToBeSet != Exit)
    {
        return false;
    }

    State = StateToBeSet;
    return true;
}

int ActionNode::GetType()
{
    // Lock acquistion

    return ACTION;
}
