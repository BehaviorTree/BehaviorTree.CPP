#include <decorator_negation_node.h>


BT::DecoratorNegationNode::DecoratorNegationNode(std::string name) : ControlNode::ControlNode(name)
{
    // thread_ start
    thread_ = boost::thread(&DecoratorNegationNode::Exec, this);
}

BT::DecoratorNegationNode::~DecoratorNegationNode() {}

void BT::DecoratorNegationNode::Exec()
{


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

                // 3) if the child state is a success:
                if(children_states_[0] == BT::SUCCESS)
                {
                    // 3.1) the node state is equal to failure since I am negating the status
                    SetNodeState(BT::FAILURE);

                    // 3.2) resetting the state;
                    WriteState(BT::IDLE);

                    std::cout << get_name() << " returning " << BT::FAILURE << "!" << std::endl;
                }
                else if(children_states_[0] == BT::FAILURE)
                {
                    // 4.1) the node state is equal to success since I am negating the status
                    SetNodeState(BT::SUCCESS);

                    // 4.2) state reset;
                    WriteState(BT::IDLE);

                    std::cout << get_name() << " returning " << BT::SUCCESS << "!" << std::endl;

                } else
                // 5) if the child state is  running
                {
                    // 5.1) the node state is equal to running
                    SetNodeState(BT::RUNNING);

                    // 5.2) state reset;
                    WriteState(BT::IDLE);
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

int BT::DecoratorNegationNode::DrawType()
{
    // Lock acquistion

    return BT::DECORATOR;
}


void BT::DecoratorNegationNode::AddChild(BT::TreeNode* child)
{
    // Checking if the child is not already present

        if (children_nodes_.size() > 0)
        {
            throw BehaviorTreeException("Decorators can have only one child");
        }


    children_nodes_.push_back(child);
    children_states_.push_back(BT::IDLE);
}
