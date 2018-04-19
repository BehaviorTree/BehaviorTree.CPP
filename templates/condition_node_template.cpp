#include <HEADER.h>


BT::CLASSNAME::CONSTRUCTOR(std::string name) : ConditionNode::ConditionNode(name)
{

}

BT::CLASSNAME::~CONSTRUCTOR() {}

BT::ReturnStatus BT::CLASSNAME::Tick()
{
        // Condition checking and state update

        if (/*CONDITION TO CHECK*/)
        {
            SetStatus(BT::SUCCESS);
            return BT::SUCCESS;
        }
        else
        {
            SetStatus(BT::FAILURE);
            return BT::FAILURE;
        }
}

