# Wraping legacy code

In this tutorial we will see how to deal with legacy code that was not meant to be used
with BehaviorTree.CPP.

Your class might look like this one:

``` C++
// This is my custom type.
struct Point3D { double x,y,z; };

// We want to create an ActionNode to calls method MyLegacyMoveTo::go
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

## C++ code

As usuall, we need to implement the template specialization of `convertFromString`.

``` C++
namespace BT
{
    template <> Point3D convertFromString(StringView key)
    {
        // three real numbers separated by semicolons
        auto parts = BT::splitString(key, ';');
        if (parts.size() != 3)
        {
            throw RuntimeError("invalid input)");
        }
        else{
            Point3D output;
            output.x  = convertFromString<double>(parts[0]);
            output.y  = convertFromString<double>(parts[1]);
            output.z  = convertFromString<double>(parts[2]);
            return output;
        }
    }
} // end anmespace BT
```

To wrap the method `MyLegacyMoveTo::go`, we may use a __lambda or std::bind__ 
to create a funtion pointer and `SimpleActionNode`.

```C++
static const char* xml_text = R"(

 <root>
     <BehaviorTree>
        <MoveTo  goal="-1;3;0.5" />
     </BehaviorTree>
 </root>
 )";

int main()
{
    using namespace BT;

    MyLegacyMoveTo move_to;

    // Here we use a lambda that captures the reference of move_to
    auto MoveToWrapperWithLambda = [&move_to](TreeNode& parent_node) -> NodeStatus
    {
        Point3D goal;
        // thanks to paren_node, you can access easily the input and output ports.
        parent_node.getInput("goal", goal);

        bool res = move_to.go( goal );
        // convert bool to NodeStatus
        return res ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    };

    BehaviorTreeFactory factory;

    // Register the lambda with BehaviorTreeFactory::registerSimpleAction

    PortsList ports = { BT::InputPort<Point3D>("goal") };
    factory.registerSimpleAction("MoveTo", MoveToWrapperWithLambda, ports );

    auto tree = factory.createTreeFromText(xml_text);

    tree.root_node->executeTick();

    return 0;
}

/* Expected output:

Going to: -1.000000 3.000000 0.500000

*/
```
 



