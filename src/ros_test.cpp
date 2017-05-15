#include "behavior_tree.h"


int main(int argc, char **argv)
{
    ros::init(argc, argv, "BehaviorTree");
    try
    {
        int TickPeriod_milliseconds = 1000;

        BT::ROSAction* action = new BT::ROSAction("action");
        BT::ROSCondition* condition = new BT::ROSCondition("condition");


        BT:: SequenceNode* sequence1 = new BT::SequenceNode("seq1");
        //SequenceStarNode* sequence1 = new SequenceStarNode("seq1");

        sequence1->AddChild(condition);
        sequence1->AddChild(action);

        Execute(sequence1, TickPeriod_milliseconds);//from BehaviorTree.cpp

}
    catch (BT::BehaviorTreeException& Exception)
    {
        std::cout << Exception.what() << std::endl;
    }

return 0;
}


