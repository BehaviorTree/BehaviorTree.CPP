# Sequences and AsyncActionNode

The next example shows the difference between a `SequenceNode` and a 
`ReactiveSequence`.

An Asynchronous Action has it's own thread. This allows the user to 
use blocking functions but to return the flow of execution 
to the tree.

```C++

// Custom type
struct Pose2D
{
    double x, y, theta;
};

class MoveBaseAction : public AsyncActionNode
{
  public:
    MoveBaseAction(const std::string& name, const NodeConfiguration& config)
      : AsyncActionNode(name, config)
    { }

    static PortsList providedPorts()
    {
        return{ InputPort<Pose2D>("goal") };
    }

    NodeStatus tick() override;

    // This overloaded method is used to stop the execution of this node.
    void halt() override
    {
        _halt_requested.store(true);
    }

  private:
    std::atomic_bool _halt_requested;
};

//-------------------------

NodeStatus MoveBaseAction::tick()
{
    Pose2D goal;
    if ( !getInput<Pose2D>("goal", goal))
    {
        throw RuntimeError("missing required input [goal]");
    }

    printf("[ MoveBase: STARTED ]. goal: x=%.f y=%.1f theta=%.2f\n", 
           goal.x, goal.y, goal.theta);

    _halt_requested.store(false);
    int count = 0;

    // Pretend that "computing" takes 250 milliseconds.
    // It is up to you to check periodicall _halt_requested and interrupt
    // this tick() if it is true.
    while (!_halt_requested && count++ < 25)
    {
        SleepMS(10);
    }

    std::cout << "[ MoveBase: FINISHED ]" << std::endl;
    return _halt_requested ? NodeStatus::FAILURE : NodeStatus::SUCCESS;
}
```

The method `MoveBaseAction::tick()` is executed in a thread different from the 
main thread that invoked `MoveBaseAction::executeTick()`.

__You are responsible__ for the implementation of a valid __halt()__ functionality.

The user must also implement `convertFromString<Pose2D>(StringView)`,
as shown in the previous tutorial.


## Sequence VS ReactiveSequence

The following example should use a simple `SequenceNode`.

```XML hl_lines="3"
 <root>
     <BehaviorTree>
        <Sequence>
            <BatteryOK/>
            <SaySomething   message="mission started..." />
            <MoveBase       goal="1;2;3"/>
            <SaySomething   message="mission completed!" />
        </Sequence>
     </BehaviorTree>
 </root>
```

```C++
int main()
{
    using namespace DummyNodes;

    BehaviorTreeFactory factory;
    factory.registerSimpleCondition("BatteryOK", std::bind(CheckBattery));
    factory.registerNodeType<MoveBaseAction>("MoveBase");
    factory.registerNodeType<SaySomething>("SaySomething");

    auto tree = factory.createTreeFromText(xml_text);

    NodeStatus status;

    std::cout << "\n--- 1st executeTick() ---" << std::endl;
    status = tree.root_node->executeTick();

    SleepMS(150);
    std::cout << "\n--- 2nd executeTick() ---" << std::endl;
    status = tree.root_node->executeTick();

    SleepMS(150);
    std::cout << "\n--- 3rd executeTick() ---" << std::endl;
    status = tree.root_node->executeTick();

    std::cout << std::endl;

    return 0;
}
```

Expected output:

``` 
    --- 1st executeTick() ---
    [ Battery: OK ]
    Robot says: "mission started..."
    [ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00

    --- 2nd executeTick() ---
    [ MoveBase: FINISHED ]

    --- 3rd executeTick() ---
    Robot says: "mission completed!"
```

You may have noticed that when `executeTick()` was called, `MoveBase` returned
__RUNNING__ the 1st and 2nd time, and eventually __SUCCESS__ the 3rd time.

`BatteryOK` is executed only once. 

If we use a `ReactiveSequence` instead, when the child `MoveBase` returns RUNNING,
the sequence is restarted and the condition `BatteryOK` is executed __again__.

If, at any point, `BatteryOK` returned __FAILURE__, the `MoveBase` action
would be _interrupted_ (_halted_, to be specific).

```XML hl_lines="3"
 <root>
     <BehaviorTree>
        <ReactiveSequence>
            <BatteryOK/>
            <SaySomething   message="mission started..." />
            <MoveBase       goal="1;2;3"/>
            <SaySomething   message="mission completed!" />
        </ReactiveSequence>
     </BehaviorTree>
 </root>
```


Expected output:

``` 
    --- 1st executeTick() ---
    [ Battery: OK ]
    Robot says: "mission started..."
    [ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00

    --- 2nd executeTick() ---
    [ Battery: OK ]
    [ MoveBase: FINISHED ]

    --- 3rd executeTick() ---
    [ Battery: OK ]
    Robot says: "mission completed!"
```



