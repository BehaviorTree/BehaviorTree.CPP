#include <ConditionNode.h>

using namespace BT;

ConditionNode::ConditionNode(std::string Name) : LeafNode::LeafNode(Name)
{
    Type = Condition;
}

ConditionNode::~ConditionNode() {}

bool ConditionNode::Halt() { return true;}

bool ConditionNode::WriteState(NodeState StateToBeSet)
{

    if(StateToBeSet != Idle)
    {
        SetColorState(StateToBeSet);
    }
    // Lock acquistion
    boost::lock_guard<boost::mutex> LockGuard(StateMutex);

    State = StateToBeSet;
    return true;
}
int ConditionNode::GetType()
{
    // Lock acquistion

    return CONDITION;
}
