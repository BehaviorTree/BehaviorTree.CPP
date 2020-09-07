# Pass additional arguments during initialization and/or construction

In every single example we explored so far we were "forced" to provide a
constructor with the following signature

```C++
    MyCustomNode(const std::string& name, const NodeConfiguration& config);

```

In same cases, it is desirable to pass to the constructor of our class 
additional arguments, parameters, pointers, references, etc.

**Many people use blackboards to do that: this is not recomendable.**

We will just use the word _"arguments"_ for the rest of the tutorial.

Even if, theoretically, these arguments **could** be passed using Input Ports,
that would be the wrong way to do it if:

- The arguments are known at _deployment-time_.
- The arguments don't change at _run-time_.
- The arguments don't need to be set from the _XML_.

If all these conditions are met, using ports or the blackboard is cumbersome and highly discouraged.

## Method 1: register a custom builder

Consider the following custom node called **Action_A**.

We want to pass three additional arguments; they can be arbitrarily complex objects,
you are not limited to built-in types.

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

    // this example doesn't require any port
    static PortsList providedPorts() { return {}; }

    // tick() can access the private members
    NodeStatus tick() override;

private:
    int _arg1;
    double _arg2;
    std::string _arg3;
};
```

This node should be registered as shown further:

```C++
BehaviorTreeFactory factory;

// A node builder is a functor that creates a std::unique_ptr<TreeNode>.
// Using lambdas or std::bind, we can easily "inject" additional arguments.
NodeBuilder builder_A =
   [](const std::string& name, const NodeConfiguration& config)
{
    return std::make_unique<Action_A>( name, config, 42, 3.14, "hello world" );
};

// BehaviorTreeFactory::registerBuilder is a more general way to
// register a custom node.
factory.registerBuilder<Action_A>( "Action_A", builder_A);

// Register more custom nodes, if needed.
// ....

// The rest of your code, where you create and tick the tree, goes here.
// ....
```

## Method 2: use an init method

Alternatively, you may call an init method before ticking the tree.

```C++

class Action_B: public SyncActionNode
{

public:
    // The constructor looks as usual.
    Action_B(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name, config) {}

    // We want this method to be called ONCE and BEFORE the first tick()
    void init( int arg1, double arg2, const std::string& arg3 )
    {
        _arg1 = (arg1);
        _arg2 = (arg2);
        _arg3 = (arg3);
    }

    // this example doesn't require any port
    static PortsList providedPorts() { return {}; }

    // tick() can access the private members
    NodeStatus tick() override;

private:
    int _arg1;
    double _arg2;
    std::string _arg3;
};
```

The way we register and initialize Action_B is slightly different:

```C++

BehaviorTreeFactory factory;

// The regitration of  Action_B is done as usual, but remember
// that we still need to call Action_B::init()
factory.registerNodeType<Action_B>( "Action_B" );

// Register more custom nodes, if needed.
// ....

// Create the whole tree
auto tree = factory.createTreeFromText(xml_text);

// Iterate through all the nodes and call init() if it is an Action_B
for( auto& node: tree.nodes )
{
    // Not a typo: it is "=", not "=="
    if( auto action_B = dynamic_cast<Action_B*>( node.get() ))
    {
        action_B->init( 42, 3.14, "hello world");
    }
}

// The rest of your code, where you tick the tree, goes here.
// ....
```





