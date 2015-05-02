#include <Actions/ActionNodeTemplate.h>

using namespace BT;

ActionNodeTemplate::ActionNodeTemplate(std::string Name) : ActionNode::ActionNode(Name)
{
    Type = Action;
    status = Failure;
    time = 1;
    // Thread start
    Thread = boost::thread(&ActionNodeTemplate::Exec, this);
}

ActionNodeTemplate::~ActionNodeTemplate() {}

void ActionNodeTemplate::Exec()
{



    while(true)
    {

        // Waiting for a tick to come
        Semaphore.Wait();

        if(ReadState() == Exit)
        {    //SetColorState(Idle);

            // The behavior tree is going to be destroied
            return;
        }

        // Running state
        SetNodeState(Running);
        std::cout << Name << " returning " << Running << "!" << std::endl;

        // Perform action...

        while(ReadState() == Running)
        {
            /*
                HERE THE CODE TO EXECUTE AS LONG AS
                THE BEHAVIOR TREE DOES NOT HALT THE ACTION
            */


            //If the action Succeeded
             setStatus(Success);
           //If the action Failed
             setStatus(Failure);

        }

        if(ReadState() == Exit)
        {
            // The behavior tree is going to be destroied
            return;
        }


        else
        {
            // trying to set the outcome state:
            if (WriteState(status) != true)
            {
                // meanwhile, my father halted me!
                std::cout << Name << " Halted!" << std::endl;

                // Resetting the state
                WriteState(Idle);

                // Next loop
                continue;
            }

            std::cout << Name << " returning " << status << "!" << std::endl;
        }

        // Synchronization
        // (my father is telling me that it has read my new state)
        Semaphore.Wait();

        if(ReadState() == Exit)
        {

            // The behavior tree is going to be destroied
            return;
        }

        // Resetting the state
        WriteState(Idle);
    }
}

bool ActionNodeTemplate::Halt()
{
    // Lock acquistion
    boost::lock_guard<boost::mutex> LockGuard(StateMutex);

    // Checking for "Running" correctness
    if (State != Running)
    {
        return false;
    }

    State = Halted;
    return true;
}


void ActionNodeTemplate::SetStatus(NodeState status){
    this->status = status;
}

void ActionNodeTemplate::SetTime(int time){
    this->time = time;
}
