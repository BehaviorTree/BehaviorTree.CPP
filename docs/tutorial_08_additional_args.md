# Custom initialization and/or construction

In every single example we explored so far we were "forced" to provide a
constructor with the following signature

```C++
    MyCustomNode(const std::string& name, const NodeConfiguration& config);

```

In same cases, it is desirable to pass to the constructor of our class 
additional arguments, parameters, pointers, references, etc.

We will just use the word _"parameter"_ for the rest of the tutorial.

Even if, theoretically, these parameters can be passed using Input Ports, 
that would be the wrong way to do it if:

- The parameters are known at _deployment-time_.
- The parameters don't change at _run-time_.
- The parameters don't need to be from the _XML_.

If all these conditions are met, using ports is just cumbersome and highly discouraged.

## The C++ example

Next, we can see two alternative ways to pass parameters to a class: 
either as arguments of the constructor of the class or in an `init()` method.

```C++
// Action_A has a different constructor than the default one.
class Action_A: public SyncActionNode
{

public:
    // additional arguments passed to the constructor
    Action_A(const std::string& name, const NodeConfiguration& config,
             int arg1, double arg2, std::string arg3 ):
        SyncActionNode(name, config),
        _arg1(arg1),
        _arg2(arg2),
        _arg3(arg3) {}

    NodeStatus tick() override
    {
        std::cout << "Action_A: " << _arg1 << " / " << _arg2 << " / " 
                  << _arg3 << std::endl;
        return NodeStatus::SUCCESS;
    }
    // this example doesn't require any port
    static PortsList providedPorts() { return {}; }

private:
    int _arg1;
    double _arg2;
    std::string _arg3;
};

// Action_B implements an init(...) method that must be called once
// before the first tick()
class Action_B: public SyncActionNode
{

public:
    Action_B(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name, config) {}

    // we want this method to be called ONCE and BEFORE the first tick()
    void init( int arg1, double arg2, std::string arg3 )
    {
        _arg1 = (arg1);
        _arg2 = (arg2);
        _arg3 = (arg3);
    }

    NodeStatus tick() override
    {
        std::cout << "Action_B: " << _arg1 << " / " << _arg2 << " / " 
                  << _arg3 << std::endl;
        return NodeStatus::SUCCESS;
    }
    // this example doesn't require any port
    static PortsList providedPorts() { return {}; }

private:
    int _arg1;
    double _arg2;
    std::string _arg3;
};
```

The way we register and initialize them in our `main` is slightly different.


```C++
static const char* xml_text = R"(

 <root >
     <BehaviorTree>
        <Sequence>
            <Action_A/>
            <Action_B/>
        </Sequence>
     </BehaviorTree>
 </root>
 )";

int main()
{
    BehaviorTreeFactory factory;

    // A node builder is nothing more than a function pointer to create a 
    // std::unique_ptr<TreeNode>.
    // Using lambdas or std::bind, we can easily "inject" additional arguments.
    NodeBuilder builder_A = 
       [](const std::string& name, const NodeConfiguration& config)
    {
        return std::make_unique<Action_A>( name, config, 42, 3.14, "hello world" );
    };

    // BehaviorTreeFactory::registerBuilder is a more general way to
    // register a custom node. 
    factory.registerBuilder<Action_A>( "Action_A", builder_A);

    // The regitration of  Action_B is done as usual, but remember 
    // that we still need to call Action_B::init()
    factory.registerNodeType<Action_B>( "Action_B" );

    auto tree = factory.createTreeFromText(xml_text);

    // Iterate through all the nodes and call init() if it is an Action_B
    for( auto& node: tree.nodes )
    {
        if( auto action_B_node = dynamic_cast<Action_B*>( node.get() ))
        {
            action_B_node->init( 69, 9.99, "interesting_value" );
        }
    }

    tree.root_node->executeTick();

    return 0;
}

   
/* Expected output:

    Action_A: 42 / 3.14 / hello world
    Action_B: 69 / 9.99 / interesting_value
*/

```





