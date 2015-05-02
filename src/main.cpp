#include "BehaviorTree.h"

using namespace BT;

int main(int argc, char **argv)
{
    try
    {
        int TickPeriod_milliseconds = 1000;

        ActionTestNode* action1 = new ActionTestNode("A1");
        ActionTestNode* action2 = new ActionTestNode("A2");

        ConditionTestNode* condition1 = new ConditionTestNode("C1");
        DecoratorNegationNode* dec = new DecoratorNegationNode("dec");

        action1->SetTime(5);
        SequenceStarNode* sequence1 = new SequenceStarNode("seq1");

        condition1->SetBehavior(Failure);
        dec->AddChild(action1);
        sequence1->AddChild(dec);
        sequence1->AddChild(action2);

        Execute(sequence1, TickPeriod_milliseconds);//from BehaviorTree.cpp
}
    catch (BehaviorTreeException& Exception)
    {
        std::cout << Exception.what() << std::endl;
    }

return 0;
}


