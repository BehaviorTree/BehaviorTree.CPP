# Async Actions using Coroutines

BehaviorTree.CPP provides two easy-to-use abstractions to create an 
asynchronous Action, i.e. those actions which:

- Take a long time to be concluded.
- May return "RUNNING".
- Can be __halted__.

The first class is a __AsyncActionNode__ that executes the tick() method in a
_separate thread_.

In this tutorial, we introduce the __CoroActionNode__, a different action that uses
[coroutines](https://www.geeksforgeeks.org/coroutines-in-c-cpp/) 
to achieve similar results.

The main reason is that Coroutines do not spawn a new thread and are much more efficient.
Furthermore, you don't need to worry about thread-safety in your code...

In Coroutines, the user should explicitly call a __yield__ method when 
he/she wants the execution of the Action to be suspended.

`CoroActionNode` wraps this `yield` function into a convenient method 
`setStatusRunningAndYield()`. 

## The C++ source example

The next example can be used as a "template" for your own implementation.


``` c++

typedef std::chrono::milliseconds Milliseconds;

class MyAsyncAction: public CoroActionNode
{
  public:
    MyAsyncAction(const std::string& name):
        CoroActionNode(name, {})
    {}

  private:
    // This is the ideal skeleton/template of an async action:
    //  - A request to a remote service provider.
    //  - A loop where we check if the reply has been received.
    //  - You may call setStatusRunningAndYield() to "pause".
    //  - Code to execute after the reply.
    //  - A simple way to handle halt().
    NodeStatus tick() override
    {
        std::cout << name() <<": Started. Send Request to server." << std::endl;

        TimePoint initial_time = Now();
        TimePoint time_before_reply = initial_time + Milliseconds(100);

        int count = 0;
        bool reply_received = false;

        while( !reply_received )
        {
            if( count++ == 0)
            {
                // call this only once
                std::cout << name() <<": Waiting Reply..." << std::endl;
            }
            // pretend that we received a reply
            if( Now() >= time_before_reply )
            {
                reply_received = true;
            }

            if( !reply_received )
            {
                // set status to RUNNING and "pause/sleep"
                // If halt() is called, we will NOT resume execution
                setStatusRunningAndYield();
            }
        }

        // This part of the code is never reached if halt() is invoked,
        // only if reply_received == true;
        std::cout << name() <<": Done. 'Waiting Reply' loop repeated "
                  << count << " times" << std::endl;
        cleanup(false);
        return NodeStatus::SUCCESS;
    }

    // you might want to cleanup differently if it was halted or successful
    void cleanup(bool halted)
    {
        if( halted )
        {
            std::cout << name() <<": cleaning up after an halt()\n" << std::endl;
        }
        else{
            std::cout << name() <<": cleaning up after SUCCESS\n" << std::endl;
        }
    }

    void halt() override
    {
        std::cout << name() <<": Halted." << std::endl;
        cleanup(true);
        // Do not forget to call this at the end.
        CoroActionNode::halt();
    }

    Timepoint Now()
    { 
        return std::chrono::high_resolution_clock::now(); 
    };
};

```

As you may have noticed, the action "pretends" to wait for a request message;
the latter will arrive after _100 milliseconds_.

To spice things up, we create a Sequence with two actions, but the entire 
sequence will be halted by a timeout after _150 millisecond_.

```XML
 <root >
     <BehaviorTree>
        <Timeout msec="150">
            <SequenceStar name="sequence">
                <MyAsyncAction name="action_A"/>
                <MyAsyncAction name="action_B"/>
            </SequenceStar>
        </Timeout>
     </BehaviorTree>
 </root>

```

No surprises in the `main()`... 

``` c++
int main()
{
    // Simple tree: a sequence of two asycnhronous actions,
    // but the second will be halted because of the timeout.

    BehaviorTreeFactory factory;
    factory.registerNodeType<MyAsyncAction>("MyAsyncAction");

    auto tree = factory.createTreeFromText(xml_text);

    //---------------------------------------
    // keep executin tick until it returns etiher SUCCESS or FAILURE
    while( tree.root_node->executeTick() == NodeStatus::RUNNING)
    {
        std::this_thread::sleep_for( Milliseconds(10) );
    }
    return 0;
}

/* Expected output:

action_A: Started. Send Request to server.
action_A: Waiting Reply...
action_A: Done. 'Waiting Reply' loop repeated 11 times
action_A: cleaning up after SUCCESS

action_B: Started. Send Request to server.
action_B: Waiting Reply...
action_B: Halted.
action_B: cleaning up after an halt()

*/
```
