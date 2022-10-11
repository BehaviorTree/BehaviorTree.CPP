#include "behaviortree_cpp_v3/bt_factory.h"

using namespace BT;

/**
 * In this tutorial, we demonstrate how to use the CoroActionNode, which
 * should be preferred over AsyncActionNode when the functions you
 * use are non-blocking.
 *
 */

class MyAsyncAction : public CoroActionNode
{
public:
  MyAsyncAction(const std::string& name) : CoroActionNode(name, {})
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
    std::cout << name() << ": Started. Send Request to server." << std::endl;

    auto Now = []() { return std::chrono::high_resolution_clock::now(); };

    TimePoint initial_time = Now();
    TimePoint time_before_reply = initial_time + std::chrono::milliseconds(100);

    int count = 0;
    bool reply_received = false;

    while (!reply_received)
    {
      if (count++ == 0)
      {
        // call this only once
        std::cout << name() << ": Waiting Reply..." << std::endl;
      }
      // pretend that we received a reply
      if (Now() >= time_before_reply)
      {
        reply_received = true;
      }

      if (!reply_received)
      {
        // set status to RUNNING and "pause/sleep"
        // If halt() is called, we will not resume execution (stack destroyed)
        setStatusRunningAndYield();
      }
    }

    // This part of the code is never reached if halt() is invoked,
    // only if reply_received == true;
    std::cout << name() << ": Done. 'Waiting Reply' loop repeated " << count << " times"
              << std::endl;
    cleanup(false);
    return NodeStatus::SUCCESS;
  }

  // you might want to cleanup differently if it was halted or successful
  void cleanup(bool halted)
  {
    if (halted)
    {
      std::cout << name() << ": cleaning up after an halt()\n" << std::endl;
    }
    else
    {
      std::cout << name() << ": cleaning up after SUCCESS\n" << std::endl;
    }
  }
  void halt() override
  {
    std::cout << name() << ": Halted." << std::endl;
    cleanup(true);
    // Do not forget to call this at the end.
    CoroActionNode::halt();
  }
};

// clang-format off
static const char* xml_text = R"(

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
 )";

// clang-format on

int main()
{
  // Simple tree: a sequence of two asycnhronous actions,
  // but the second will be halted because of the timeout.

  BehaviorTreeFactory factory;
  factory.registerNodeType<MyAsyncAction>("MyAsyncAction");

  auto tree = factory.createTreeFromText(xml_text);

  //---------------------------------------
  tree.tickRootWhileRunning();

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
