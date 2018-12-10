# Async Actions using Coroutines

BehaviorTree.CPP provides two easy-to-use abstractions to create an 
asynchronous Action, i.e those actions which:


- Take a long time to be concluded.
- May return "RUNNING".
- Can be __halted__.

The first class is __AsyncActionNode__, that execute the tick() method in a
separate thread.

In this tutorial we present __CoroActionNode__, a different class that uses
coroutines to achieve a similar results.

Coroutines do not spawn a new thread and are much more efficient.

The user should explicitly call a __yield__ method where he wants to execution
of the Action to be suspended.

In this tutorial we will see how it works with a very simple example that 
you can use as template of your own implementation.



``` c++
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

```

To keep the rest of the example simple, we use create a trivial tree
with two actions executed in sequence, using the programmatic approach. 

``` c++
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

actionA: Started. Send Request to server.
actionA: Waiting reply.
actionA: Waiting reply.
actionA: Waiting reply.
actionA: Done.
actionB: Started. Send Request to server.
actionB: Waiting reply.
actionB: Waiting reply.
actionB: Waiting reply.
actionB: Done.
*/
```
