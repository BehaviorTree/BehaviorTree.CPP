#include <HEADER.h>

BT::CLASSNAME::CONSTRUCTOR(const std::string& name) : ConditionNode::ConditionNode(name)
{
}

BT::CLASSNAME::~CONSTRUCTOR()
{
}

BT::ReturnStatus BT::CLASSNAME::Tick()
{
    // Condition checking and state update

    if (/*CONDITION TO CHECK*/)
    {
        SetStatus(NodeStatus::SUCCESS);
        return NodeStatus::SUCCESS;
    }
    else
    {
        SetStatus(NodeStatus::FAILURE);
        return NodeStatus::FAILURE;
    }
}
