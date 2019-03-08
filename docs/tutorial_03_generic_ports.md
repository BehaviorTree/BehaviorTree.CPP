# Ports with generic types

In the previous tutorials we introduced input and output ports, where the
type of the port itself was a `std::string`.

This is the easiest port type to deal with, because any parameter passed
from the XML definition will be (obviously) a string.

Next, we will describe how to use any generic C++ type in your code.

## Parsing a string

__BehaviorTree.CPP__ supports automatic conversion of strings into common
types, such as `int`, `long`, `double`, `bool`, `NodeStatus`, etc.

But user defined types can be supported easily as well. For instance:

```C++
// We want to be able to use this custom type
struct Position2D 
{ 
  double x;
  double y; 
};
```

To parse a string into a `Position2D` we should link to a template 
specialization of `BT::convertFromString<Position2D>(StringView)`.

We can use any syntax we want; in this case, we simply separate two numbers
with a _semicolon_.


```C++
// Template specialization to converts a string to Position2D.
namespace BT
{
    template <> inline Position2D convertFromString(StringView str)
    {
        // The next line should be removed...
        printf("Converting string: \"%s\"\n", str.data() );

        // We expect real numbers separated by semicolons
        auto parts = splitString(str, ';');
        if (parts.size() != 2)
        {
            throw RuntimeError("invalid input)");
        }
        else{
            Position2D output;
            output.x     = convertFromString<double>(parts[0]);
            output.y     = convertFromString<double>(parts[1]);
            return output;
        }
    }
} // end namespace BT
```

About the previous code:

- `StringView` is just a C++11 version of [std::string_view](https://en.cppreference.com/w/cpp/header/string_view). 
   You can pass either a `std::string` or a `const char*`.
-  In production code, you would (obviously) remove the `printf` statement.
-  The library provides a simple `splitString` function. Feel free to use another
   one, like [boost::algorithm::split](onvertFromString<double>).
-  Once we split the input into single numbers, we can reuse the specialization 
   `convertFromString<double>()`.  
   
## Example

Similarly to the previous tutorial, we can create two custom Actions,
one will write into a port and the other will read from a port.


```C++

class CalculateGoal: public SyncActionNode
{
public:
    CalculateGoal(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name,config)
    {}

    static PortsList providedPorts()
    {
        return { OutputPort<Position2D>("goal") };
    }

    NodeStatus tick() override
    {
        Position2D mygoal = {1.1, 2.3};
        setOutput<Position2D>("goal", mygoal);
        return NodeStatus::SUCCESS;
    }
};


class PrintTarget: public SyncActionNode
{
public:
    PrintTarget(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name,config)
    {}

    static PortsList providedPorts()
    {
        // Optionally, a port can have a human readable description
        const char*  description = "Simply print the goal on console...";
        return { InputPort<Position2D>("target", description) };
    }
    
    NodeStatus tick() override
    {
        auto res = getInput<Position2D>("target");
        if( !res )
        {
            throw RuntimeError("error reading port [target]:", res.error());
        }
        Position2D target = res.value();
        printf("Target positions: [ %.1f, %.1f ]\n", target.x, target.y );
        return NodeStatus::SUCCESS;
    }
};
```   

We can now connect input/output ports as usual, pointing to the same 
entry of the Blackboard.

The tree in the next example is a Sequence of 4 actions

- Store a value of `Position2D` in the entry "GoalPosition",
  using the action `CalculateGoal`.

- Call `PrintTarget`. The input "target" will be read from the Blackboard
  entry "GoalPosition".

- Use the built-in action `SetBlackboard` to write the key "OtherGoal".
  A conversion from string to `Position2D` will be done under the hood.

- Call `PrintTarget` again. The input "target" will be read from the Blackboard
  entry "OtherGoal".


```C++  
static const char* xml_text = R"(

 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
            <CalculateGoal   goal="{GoalPosition}" />
            <PrintTarget     target="{GoalPosition}" />
            <SetBlackboard   output_key="OtherGoal" value="-1;3" />
            <PrintTarget     target="{OtherGoal}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
 )";

int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerNodeType<CalculateGoal>("CalculateGoal");
    factory.registerNodeType<PrintTarget>("PrintTarget");

    auto tree = factory.createTreeFromText(xml_text);
    tree.root_node->executeTick();

/* Expected output:

    Target positions: [ 1.1, 2.3 ]
    Converting string: "-1;3"
    Target positions: [ -1.0, 3.0 ]
*/
    return 0;
}
```  






   
