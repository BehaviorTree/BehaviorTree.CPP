#include <ActionNodeExample.h>
#include <ConditionNodeExample.h>
#include "BehaviorTree.h"
#include <iostream>

using namespace BTExample;

int main(int argc, char **argv)
{
    try
    {
        int TickPeriod_milliseconds = 1000;

        ActionNodeExample* action1 = new ActionNodeExample("A1");
        ActionNodeExample* action2 = new ActionNodeExample("A2");

        ConditionNodeExample* condition1 = new ConditionNodeExample("C1");
        BT::DecoratorNegationNode* dec = new BT::DecoratorNegationNode("dec");

        action1->SetTime(5);
        BT::SequenceStarNode* sequence1 = new BT::SequenceStarNode("seq1");

        condition1->SetBehavior(BT::Failure);
        dec->AddChild(action1);
        sequence1->AddChild(dec);
        sequence1->AddChild(action2);

        Execute(sequence1, TickPeriod_milliseconds);//from BehaviorTree.cpp
    }
    catch (BT::BehaviorTreeException& Exception)
    {
        std::cout << Exception.what() << std::endl;
    }

	return 0;
}


