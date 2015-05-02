#include "BehaviorTree.h"

using namespace BT;

int main(int argc, char **argv)
{
    try
    {
        int TickPeriod_milliseconds = 1000;

        ActionTestNode* test1 = new ActionTestNode("A1");
        ActionTestNode*test2 = new ActionTestNode("A2");
        ActionTestNode*test3 = new ActionTestNode("A3");
        ActionTestNode*test4 = new ActionTestNode("A4");

        SequenceStarNode* sequence1 = new SequenceStarNode("seq1");
        SelectorStarNode* selector1 = new SelectorStarNode("sel1");
        SelectorStarNode* selector2 = new SelectorStarNode("sel12");

        DecoratorRetryNode* dec = new DecoratorRetryNode("retry",2);
        SequenceStarNode* root = new SequenceStarNode("root");

        test1->SetBehavior(Success);
        test1->SetTime(3);
        test2->SetBehavior(Success);
        test2->SetTime(2);
        test3->SetBehavior(Failure);
        test4->SetBehavior(Success);

        selector1->AddChild(test1);
        selector1->AddChild(test2);

        dec->AddChild(test1);

        root->AddChild(test3);
        root->AddChild(selector1);

        Execute(root, TickPeriod_milliseconds);//from BehaviorTree.cpp
}
    catch (BehaviorTreeException& Exception)
    {
        std::cout << Exception.what() << std::endl;
    }

return 0;
}


