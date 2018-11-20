# Wrap legacy code

In this tutorial we see how to deal with legacy code that was not meant to be used
with BehaviorTree.CPP.

Let's start supposing that this is my class.

``` c++
// This is my custom type.
struct Point3D { double x,y,z; };

class MyLegacyMoveTo
{
public:
    bool go(Point3D goal)
    {
        printf("Going to: %f %f %f\n", goal.x, goal.y, goal.z);
        return true; // true means success in my legacy code
    }
};
```

We want to create an ActionNode called "MoveTo" that invokes the method __MyLegacyMoveTo::go()__.

The final goal is to be able to use this ActionNode in a tree like this one:

``` XML
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
            <MoveTo  goal="-1;3;0.5" />
            <MoveTo  goal="${myGoal}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
``` 

The first thing that we need to do is to allow our library to convert
a NodeParameter (that is
nothing more than a pair of strings representing key/value) into a Point3D.

As we did in a previous tutorial, we should implement a template specialization
for __convertFromString__.

Our particular string representation of a Point3D consists in three semicolon-separated
numbers, representing __x, y and z_.


``` c++
namespace BT
{
template <> Point3D convertFromString(const StringView& key)
{
    // three real numbers separated by semicolons
    auto parts = BT::splitString(key, ';');
    if (parts.size() != 3)
    {
        throw std::runtime_error("invalid input)");
    }
    else
    {
        Point3D output;
        output.x  = convertFromString<double>(parts[0]);
        output.y  = convertFromString<double>(parts[1]);
        output.z  = convertFromString<double>(parts[2]);
        return output;
    }
}
}
```

Finally, we can use a __C++11 lambda__ (or, alternatively, __std::bind__) to wrap
out method into a function with the right signature.

``` c++
int main()
{
    using namespace BT;

    MyLegacyMoveTo move_to;

    // Here we use a lambda that captures the reference of move_to
    auto MoveToWrapperWithLambda = [&move_to](TreeNode& parent_node) -> NodeStatus
    {
        Point3D goal;
        // thanks to paren_node, you can access easily the NodeParameters and the blackboard
        parent_node.getParam("goal", goal);

        bool res = move_to.go( goal );
        // convert bool to NodeStatus
        return res ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    };

    BehaviorTreeFactory factory;
    factory.registerSimpleAction("MoveTo", MoveToWrapperWithLambda);

    auto blackboard = Blackboard::create<BlackboardLocal>();
    auto tree = buildTreeFromText(factory, xml_text, blackboard);

    // We set the entry "myGoal" in the blackboard.
    Point3D my_goal = {3,4,5};
    blackboard->set("myGoal", my_goal);

    NodeStatus status = NodeStatus::RUNNING;
    while (status == NodeStatus::RUNNING)
    {
        status = tree.root_node->executeTick();
    }
    return 0;
}

/* Expected output:

Going to: -1.000000 3.000000 0.500000
Going to: 3.000000 4.000000 5.000000

The first MoveTo read the parameter from the string "-1;3;0.5"
whilst the second from the blackboard, that contains a copy of the Point3D my_goal.

*/
```


The functor we are passing to __SimpleActionNode__ requires the following signature:

     BT::NodeStatus myFunction(BT::TreeNode& parent) 
     
As a consequence, we can access a NodeParameter by

     parent.getParam()
     
or even set/get an entry of the Blackboard using

     parent.blackboard()





