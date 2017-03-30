
#include "actions/action_test_node.h"
#include "conditions/condition_test_node.h"
#include "behavior_tree.h"
#include <iostream>


int main(int argc, char **argv)
{
    try
    {
        int TickPeriod_milliseconds = 1000;

        BT::ActionTestNode* action1 = new BT::ActionTestNode("Action1");
        BT::ConditionTestNode* condition1 = new BT::ConditionTestNode("Condition1");
        action1->set_time(5);
        BT::SequenceNodeWithMemory* sequence1 = new BT::SequenceNodeWithMemory("seq1");

        condition1->set_boolean_value(true);
        sequence1->AddChild(condition1);
        sequence1->AddChild(action1);

        Execute(sequence1, TickPeriod_milliseconds);//from BehaviorTree.cpp
    }
    catch (BT::BehaviorTreeException& Exception)
    {
        std::cout << Exception.what() << std::endl;
    }

	return 0;
}


