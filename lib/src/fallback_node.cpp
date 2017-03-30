#include <fallback_node.h>


BT::FallbackNode::FallbackNode(std::string name) : ControlNode::ControlNode(name)
{

}

BT::FallbackNode::~FallbackNode() {}

BT::ReturnStatus BT::FallbackNode::Tick()
{
    {


        // gets the number of children. The number could change if, at runtime, one edits the tree.
        N_of_children_ = children_nodes_.size();

        // Routing the ticks according to the fallback node's logic:

        for (unsigned int i = 0; i < N_of_children_; i++)
        {
    /*      Ticking an action is different from ticking a condition. An action executed some portion of code in another thread.
            We want this thread detached so we can cancel its execution (when the action no longer receive ticks).
            Hence we cannot just call the method Tick() from the action as doing so will block the execution of the tree.
            For this reason if a child of this node is an action, then we send the tick using the tick engine. Otherwise we call the method Tick() and wait for the response.
    */
            if (children_nodes_[i]->get_type() == BT::ACTION_NODE)
            {
                //1) If the child i is an action, read its state.
                child_i_status_ = children_nodes_[i]->get_status();

                if (child_i_status_ == BT::IDLE || child_i_status_ == BT::HALTED)
                {
                    //1.1) If the action status is not running, the sequence node sends a tick to it.
                    DEBUG_STDOUT(get_name() << "NEEDS TO TICK " << children_nodes_[i]->get_name());
                    children_nodes_[i]->tick_engine.Tick();

                    //waits for the tick to arrive to the child
                    do
                    {
                        child_i_status_ = children_nodes_[i]->get_status();
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    }
                    while(child_i_status_ != BT::RUNNING && child_i_status_ != BT::SUCCESS && child_i_status_ != BT::FAILURE);
                }

            }
            else
            {
                // 2) if it's not an action:
                // Send the tick and wait for the response;
                child_i_status_ = children_nodes_[i]->Tick();
            }
            //Ponderate on which status to send to the parent
            if(child_i_status_ != BT::FAILURE)
            {

                if(child_i_status_ == BT::SUCCESS)
                {

                    children_nodes_[i]->set_status(BT::IDLE);//the child goes in idle if it has returned success.
                }


                // If the  child status is not failure, halt the next children and return the status to your parent.
                DEBUG_STDOUT(get_name() << " is HALTING children from " << (i+1));
                HaltChildren(i+1);
                set_status(child_i_status_);
                return child_i_status_;
            }
            else
            {//the child returned failure.
                children_nodes_[i]->set_status(BT::IDLE);

                if(i == N_of_children_ - 1)
                {  // If the  child status is failure, and it is the last child to be ticked, then the sequence has failed.
                    set_status(BT::FAILURE);
                    return BT::FAILURE;
                }
            }
        }


    }

}

int BT::FallbackNode::DrawType()
{
    // Lock acquistion

    return BT::SELECTOR;
}
