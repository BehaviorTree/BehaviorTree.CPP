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
            set_status(BT::SUCCESS);
            return BT::SUCCESS;
        }
        else
        {
            set_status(BT::FAILURE);
            return BT::FAILURE;
        }
}

