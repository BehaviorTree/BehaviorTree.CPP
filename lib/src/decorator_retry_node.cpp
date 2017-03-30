#include <decorator_retry_node.h>


BT::DecoratorRetryNode::DecoratorRetryNode(std::string name, unsigned int NTries) : ControlNode::ControlNode(name)
{
    // thread_ start
    NTries_ = NTries;
    thread_ = boost::thread(&DecoratorRetryNode::Exec, this);
}

BT::DecoratorRetryNode::~DecoratorRetryNode() {}

void BT::DecoratorRetryNode::Exec()
{
    int i;

    // Waiting for the first tick to come
    tick_engine.wait();

    // Vector size initialization
    N_of_children_ = children_nodes_.size();

    // Simulating a tick for myself
    tick_engine.tick();

    while(true)
    {
        // Waiting for a tick to come
        tick_engine.wait();

        if(ReadState() == BT::EXIT)
        {
            // The behavior tree is going to be destroied
            return;
        }

        // Checking if i was halted
        if (ReadState() != BT::HALTED)
        {
            // If not, the children can be ticked
            std::cout << get_name() << " ticked, ticking children..." << std::endl;

            TryIndx_ = 0;
            // For each child:
            //for (i = 0; i<M; i++)
            {
                if (children_nodes_[0]->get_type() == BT::ACTION_NODE)
                {
                    // 1) if it's an action:
                    // 1.1) read its state;
                    ReturnStatus ActionState = children_nodes_[0]->ReadState();

                    if (ActionState == BT::IDLE)
                    {
                        // 1.2) if it's "Idle":
                        // 1.2.1) ticking it;
                        children_nodes_[0]->tick_engine.tick();

                        // 1.2.2) retrive its state as soon as it is available;
                        children_states_[0] = children_nodes_[0]->GetNodeState();
                    }
                    else if (ActionState == BT::RUNNING)
                    {
                        // 1.3) if it's "Running":
                        // 1.3.1) saving "Running"
                        children_states_[0] = BT::RUNNING;
                    }
                    else
                    {
                        // 1.4) if it's "Success" of "Failure" (it can't be "Halted"!):
                        // 1.2.1) ticking it;
                        children_nodes_[0]->tick_engine.tick();

                        // 1.2.2) saving the read state;
                        children_states_[0] = ActionState;
                    }
                }
                else
                {
                    // 2) if it's not an action:
                    // 2.1) ticking it;
                    children_nodes_[0]->tick_engine.tick();

                    // 2.2) retrive its state as soon as it is available;
                    children_states_[0] = children_nodes_[0]->GetNodeState();
                }

                // 3) if the child state is not a success:
                if(children_states_[0] == BT::SUCCESS)
                {
                    SetNodeState(BT::SUCCESS);

                    // 4.2) resetting the state;
                    WriteState(BT::IDLE);

                    std::cout << get_name() << " returning " << BT::SUCCESS << "!" << std::endl;
                }
                else
                {
                    if(children_states_[0] == BT::FAILURE)
                    {
                        children_nodes_[0]->ResetColorState();
                        TryIndx_++;

                    }

                    if(children_states_[0] == BT::FAILURE && TryIndx_ < NTries_)
                    {
                        // 3.1) the node state is equal to running since I am rerunning the child
                        SetNodeState(BT::RUNNING);
                        // 3.2) state reset;
                        WriteState(BT::IDLE);
                    }
                    else
                    {
                        SetNodeState(children_states_[0]);
                        // 3.2) state reset;
                        WriteState(BT::IDLE);
                        std::cout << get_name() << " returning " << children_states_[0] << "!" << std::endl;

                    }
                }
            }

        }
        else
        {
            // If it was halted, all the "busy" children must be halted too
            std::cout << get_name() << " halted! Halting all the children..." << std::endl;

                if (children_nodes_[0]->get_type() != BT::ACTION_NODE && children_states_[0] == BT::RUNNING)
                {
                    // if the control node was running:
                    // halting it;
                    children_nodes_[0]->Halt();

                    // sync with it (it's waiting on the semaphore);
                    children_nodes_[0]->tick_engine.tick();

                    std::cout << get_name() << " halting child  "  << "!" << std::endl;
                }
                else if (children_nodes_[0]->get_type() == BT::ACTION_NODE && children_nodes_[0]->ReadState() == BT::RUNNING)
                {
                    std::cout << get_name() << " trying halting child  "  << "..." << std::endl;

                    // if it's a action node that hasn't finished its job:
                    // trying to halt it:
                    if (children_nodes_[0]->Halt() == false)
                    {
                        // this means that, before this node could set its child state
                        // to "Halted", the child had already written the action outcome;
                        // sync with him ignoring its state;
                        children_nodes_[0]->tick_engine.tick();

                        std::cout << get_name() << " halting of child  "  << " failed!" << std::endl;
                    }

                    std::cout << get_name() << " halting of child  "  << " succedeed!" << std::endl;
                }
                else if (children_nodes_[0]->get_type() == BT::ACTION_NODE && children_nodes_[0]->ReadState() != BT::IDLE)
                {
                    // if it's a action node that has finished its job:
                    // ticking it without saving its returning state;
                    children_nodes_[0]->tick_engine.tick();
                }

                // updating its vector cell
                children_states_[0] = BT::IDLE;


            // Resetting the node state
            WriteState(BT::IDLE);
        }
    }
}

int BT::DecoratorRetryNode::DrawType()
{
    // Lock acquistion

    return BT::DECORATOR;
}

