#include "behaviortree_cpp/blackboard/blackboard_local.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace BT;

/**
 * In this first tutorial we demonstrate how use the CoroActionNode, which
 * should be preferred over AsyncActionNode.
 *
 */

class MyAsyncAction: public CoroActionNode
{
  public:
    MyAsyncAction(const std::string& name):
      CoroActionNode(name, NodeParameters())
    {}

    // This is the ideal skeleton/template of an async action:
    //  - A request to a remote service provider.
    //  - A loop where we check if the reply has been received.
    //    Call setStatusRunningAndYield() to "pause".
    //  - Code to execute after the reply.
    //  - a simple way to handle halt().

    NodeStatus tick() override
    {
        std::cout << name() <<": Started. Send Request to server." << std::endl;

        int cycle = 0;
        bool reply_received = false;

        while( !reply_received )
        {
            std::cout << name() <<": Waiting reply." << std::endl;
            reply_received = ++cycle >= 3;

            if( !reply_received )
            {
                // set status to RUNNING and "pause/sleep"
                // If halt() is called, we will not resume execution
                setStatusRunningAndYield();
            }
        }

        // this part of the code is never reached if halt() is invoked.
        std::cout << name() <<": Done." << std::endl;
        return NodeStatus::SUCCESS;
    }

    void halt() override
    {
        std::cout << name() <<": Halted. Do your cleanup here." << std::endl;

        // Do not forget to call this at the end.
        CoroActionNode::halt();
    }
};


int main()
{
    // Simple tree: a sequence of two asycnhronous actions
    BT::SequenceNode sequence_root("sequence");
    MyAsyncAction action_A("actionA");
    MyAsyncAction action_B("actionB");

    // Add children to the sequence.
    sequence_root.addChild(&action_A);
    sequence_root.addChild(&action_B);

    NodeStatus status = NodeStatus::IDLE;

    while( status != NodeStatus::SUCCESS && status != NodeStatus::FAILURE)
    {
        status = sequence_root.executeTick();

        // It is often a good idea to add a sleep here to avoid busy loops
        std::this_thread::sleep_for( std::chrono::milliseconds(1) );
    }

    return 0;
}

/* Expected output:

actionA: Started. Request service using async call
actionA: Waiting reply
actionA: Waiting reply
actionA: Waiting reply
actionA: Done
actionB: Started. Request service using async call
actionB: Waiting reply
actionB: Waiting reply
actionB: Waiting reply
actionB: Done
*/
