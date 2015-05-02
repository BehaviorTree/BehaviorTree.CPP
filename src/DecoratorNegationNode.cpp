#include <DecoratorNegationNode.h>

using namespace BT;

DecoratorNegationNode::DecoratorNegationNode(std::string Name) : ControlNode::ControlNode(Name)
{
    // Thread start
    Thread = boost::thread(&DecoratorNegationNode::Exec, this);
}

DecoratorNegationNode::~DecoratorNegationNode() {}

void DecoratorNegationNode::Exec()
{


    // Waiting for the first tick to come
    Semaphore.Wait();

    // Vector size initialization
    M = ChildNodes.size();

    // Simulating a tick for myself
    Semaphore.Signal();

    while(true)
    {
        // Waiting for a tick to come
        Semaphore.Wait();

        if(ReadState() == Exit)
        {
            // The behavior tree is going to be destroied
            return;
        }

        // Checking if i was halted
        if (ReadState() != Halted)
        {
            // If not, the children can be ticked
            std::cout << Name << " ticked, ticking children..." << std::endl;




                if (ChildNodes[0]->Type == Action)
                {
                    // 1) if it's an action:
                    // 1.1) read its state;
                    NodeState ActionState = ChildNodes[0]->ReadState();

                    if (ActionState == Idle)
                    {
                        // 1.2) if it's "Idle":
                        // 1.2.1) ticking it;
                        ChildNodes[0]->Semaphore.Signal();

                        // 1.2.2) retrive its state as soon as it is available;
                        ChildStates[0] = ChildNodes[0]->GetNodeState();
                    }
                    else if (ActionState == Running)
                    {
                        // 1.3) if it's "Running":
                        // 1.3.1) saving "Running"
                        ChildStates[0] = Running;
                    }
                    else
                    {
                        // 1.4) if it's "Success" of "Failure" (it can't be "Halted"!):
                        // 1.2.1) ticking it;
                        ChildNodes[0]->Semaphore.Signal();

                        // 1.2.2) saving the read state;
                        ChildStates[0] = ActionState;
                    }
                }
                else
                {
                    // 2) if it's not an action:
                    // 2.1) ticking it;
                    ChildNodes[0]->Semaphore.Signal();

                    // 2.2) retrive its state as soon as it is available;
                    ChildStates[0] = ChildNodes[0]->GetNodeState();
                }

                // 3) if the child state is a success:
                if(ChildStates[0] == Success)
                {
                    // 3.1) the node state is equal to failure since I am negating the status
                    SetNodeState(Failure);

                    // 3.2) resetting the state;
                    WriteState(Idle);

                    std::cout << Name << " returning " << Failure << "!" << std::endl;
                }
                else if(ChildStates[0] == Failure)
                {
                    // 4.1) the node state is equal to success since I am negating the status
                    SetNodeState(Success);

                    // 4.2) state reset;
                    WriteState(Idle);

                    std::cout << Name << " returning " << Success << "!" << std::endl;

                } else
                // 5) if the child state is  running
                {
                    // 5.1) the node state is equal to running
                    SetNodeState(Running);

                    // 5.2) state reset;
                    WriteState(Idle);
                }

        }
        else
        {
            // If it was halted, all the "busy" children must be halted too
            std::cout << Name << " halted! Halting all the children..." << std::endl;

                if (ChildNodes[0]->Type != Action && ChildStates[0] == Running)
                {
                    // if the control node was running:
                    // halting it;
                    ChildNodes[0]->Halt();

                    // sync with it (it's waiting on the semaphore);
                    ChildNodes[0]->Semaphore.Signal();

                    std::cout << Name << " halting child  "  << "!" << std::endl;
                }
                else if (ChildNodes[0]->Type == Action && ChildNodes[0]->ReadState() == Running)
                {
                    std::cout << Name << " trying halting child  "  << "..." << std::endl;

                    // if it's a action node that hasn't finished its job:
                    // trying to halt it:
                    if (ChildNodes[0]->Halt() == false)
                    {
                        // this means that, before this node could set its child state
                        // to "Halted", the child had already written the action outcome;
                        // sync with him ignoring its state;
                        ChildNodes[0]->Semaphore.Signal();

                        std::cout << Name << " halting of child  "  << " failed!" << std::endl;
                    }

                    std::cout << Name << " halting of child  "  << " succedeed!" << std::endl;
                }
                else if (ChildNodes[0]->Type == Action && ChildNodes[0]->ReadState() != Idle)
                {
                    // if it's a action node that has finished its job:
                    // ticking it without saving its returning state;
                    ChildNodes[0]->Semaphore.Signal();
                }

                // updating its vector cell
                ChildStates[0] = Idle;


            // Resetting the node state
            WriteState(Idle);
        }
    }
}

int DecoratorNegationNode::GetType()
{
    // Lock acquistion

    return DECORATOR;
}


void DecoratorNegationNode::AddChild(TreeNode* Child)
{
    // Checking if the Child is not already present

        if (ChildNodes.size() > 0)
        {
            throw BehaviorTreeException("Decorators can have only one child");
        }


    ChildNodes.push_back(Child);
    ChildStates.push_back(Idle);
}
