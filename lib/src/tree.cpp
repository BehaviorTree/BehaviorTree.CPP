#include "behavior_tree.h"


int main(int argc, char **argv)
{
    ros::init(argc, argv, "BehaviorTree");
    try
    {
        int TickPeriod_milliseconds = 1000;

        BT::ActionTestNode* action1 = new BT::ActionTestNode("Action 1");
        BT::ConditionTestNode* condition1 = new BT::ConditionTestNode("Condition 1");
        BT:: SequenceNode* sequence1 = new BT::SequenceNode("seq1");




        BT::ActionTestNode* action2 = new BT::ActionTestNode("Action 2");
        BT::ConditionTestNode* condition2 = new BT::ConditionTestNode("Condition 2");
        BT:: SequenceNode* sequence2 = new BT::SequenceNode("seq1");



        BT::ActionTestNode* action3 = new BT::ActionTestNode("Action 3");
        BT::ConditionTestNode* condition3 = new BT::ConditionTestNode("Condition 3");
        BT:: SequenceNode* sequence3 = new BT::SequenceNode("seq1");


        BT::ActionTestNode* action4 = new BT::ActionTestNode("Action 4");
        BT::ConditionTestNode* condition4 = new BT::ConditionTestNode("Condition 4");
        BT:: SequenceNode* sequence4 = new BT::SequenceNode("seq1");




        sequence1->AddChild(condition2);
        sequence1->AddChild(action1);
        sequence1->AddChild(sequence2);
        sequence1->AddChild(action3);

        sequence1->AddChild(sequence2);



        sequence2->AddChild(action2);
       // sequence2->AddChild(sequence3);
        sequence2->AddChild(condition2);


        sequence3->AddChild(condition3);
        sequence3->AddChild(action3);



//        sequence4->AddChild(condition4);
//        sequence4->AddChild(action4);

//        std::string text = "";

//        BT:: SequenceNode* root = new BT::SequenceNode("seq1");

//        for (int i = 0; i < 10; i++)
//        {
//            BT:: SequenceNode* seq = new BT::SequenceNode("seq");
//            text = "Action ";
//            text += std::to_string(i);
//            BT::ActionTestNode* action = new BT::ActionTestNode(text);
//            text = "Condition ";
//            text += std::to_string(i);
//            BT::ConditionTestNode* condition = new BT::ConditionTestNode(text);
//            seq->AddChild(condition);

//            seq->AddChild(action);

//            root->AddChild(seq);
//        }

        Execute(sequence1, TickPeriod_milliseconds);//from BehaviorTree.cpp

}
    catch (BT::BehaviorTreeException& Exception)
    {
        std::cout << Exception.what() << std::endl;
    }

return 0;
}


