#include <Conditions/ConditionNodeTemplate.h>

using namespace BT;

ConditionNodeTemplate::ConditionNodeTemplate(std::string Name) : ConditionNode::ConditionNode(Name)
{
    Type = Condition;
    // Thread start
    Thread = boost::thread(&ConditionNodeTemplate::Exec, this);
}

ConditionNodeTemplate::~ConditionNodeTemplate() {}

void ConditionNodeTemplate::Exec()
{
    while(true)
    {
	
        // Waiting for a tick to come
        Semaphore.Wait();

        if(ReadState() == Exit)
        {
            // The behavior tree is going to be destroied
            return;
        }

        // Condition checking and state update
        if (/*your_condition satisfied*/)
        {
            SetNodeState(Success);
            std::cout << "Condition" << Name << " satisfied" << "!" << std::endl;
        }
        else
        {
            SetNodeState(Failure);
            std::cout << "Condition" << Name << " not satisfied" << "!" << std::endl;

        }


        // Resetting the state
        WriteState(Idle);
    }
}

