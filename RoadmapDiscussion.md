# 1. Roadmap: input/output ports in TreeNode 
## (Updated the 2019_01_03)

One of the goals of this project is to separate the role of the __Component
Developer__ from the __Behavior Designed__ and __System Integrator__.

Rephrasing:

- Custom Actions (or, in general, custom TreeNodes) must be reusable building
blocks.

- To build a BehaviorTree out of TreeNodes, the Behavior Designer must not need to read 
nor modify the source code of the a given TreeNode.

There is a __major design flow__ that undermines this goal: the way
the BlackBoard is currently used to implement dataflow between nodes.

As described in [issue #18](https://github.com/BehaviorTree/BehaviorTree.CPP/issues/18)
there are several potential problems:

- To know which entries of the BB are read/written, you should read the source code.
- As a consequence, external tools such as __Groot__ can not know which BB entries are accessed.
- If there is a name clashing (multiple nodes use the same key for different purposes),
 the only way to fit it is modifying the source code. 

SMACH solved this problem using [input and output ports](http://wiki.ros.org/smach/Tutorials/User%20Data)
and remapping to connect them.

In the ROS community, we potentially have the same problem with topics,
but tools such as __rosinfo__ provides introspection at run-time and name
clashing is avoided using remapping.

# 2. Suggested changes to the library

Goals of the new design:

- The `TreeNodeManifest` should contain information about input and outputs ports, 
to make this information available to external tools.

- Avoid name clashing using key remapping.


## 2.1 Deprecate TreeNode::blackboard()

In version 2.x the user is allowed to read and write into any single
entry of the blackboard.

As a consequence, there is no way to introspect which entries are accessed.

For this reason, we must deprecate `TreeNode::blackboard()` and use instead
a more sensible API such as `getInput` and `setOutput`.

The latter methods should access only a limited number of entries, the __ports__.

##  2.2 NameParameters == Input Ports

In version 2.X, `NodeParameters` are a mechanism to add "arguments" to a Node.

A NodeParameter can be either:

- text that is parsed by the user's code using `convertFromString()` or 
- a "pointer" to an entry of the BB.

After few months, it became clear that the latter case is the rule rather than the exception:
in probably 80-90% of the cases, NodeParameters are passed through the BB.

Furthermore, `requiredNodeParameters` is already an effective way to 
automatically create a manifest.

For these reasons, we may consider NodeParameters a valid implementation of an
__input port__.

The implementation would still be the same, what changes is our interpretation,
i.e. NodeParameter __are__ input ports.

From a practical point of view, we should encourage the use of
`TreeNode::getParam` and deprecate `TreeNode::blackboard()::get`.

Furthermore, it makes sense, for consistency, to rename `getParam` to __getInput__.

## 2.3 Output Ports

We need to add the output ports to the TreeNodeManifest.

The static method `requiredNodeParameters` should be replaced by 
`providedPorts`:


```c++

enum class PortType { INPUT, OUTPUT, INOUT };

typedef std::unordered_map<std::string, PortType> PortsList;

// New Manifest.
struct TreeNodeManifest
{
    NodeType type;
    std::string registration_ID;
    PortsList ports;
};

// What was previously MyNode::requiredNodeParameters() becomes:
static PortsList MyNode::providedPorts();

```
 
Let's consider the problem of __remapping__.

To avoid name clashing it is sufficient to remap __only for the output ports__.

We don't need to remap input ports, because the name of the entry is 
already provided at run-time (in the XML).

From the user prospective, `TreeNode::blackboard()::set(key,value)` is replaced 
by a new method `TreeNode::setOutput(key,value)`.

__Example__:

If the remapping pair __["goal","navigation_goal"]__ is passed and the user invokes

      setOutput("goal", "kitchen");

The actual entry to be written will be the `navigation_goal`.

# 3. Further changes: NodeConfiguration

### Major (breaking) changes in the signature of TreeNodes

Since we are breaking the API, it makes sense to add another improvement that
is not backward compatible.

People want to read/write  from/to the blackboard in their constructor.
The callback `onInit()` was a workaround.

For these reasons, we propose to change the signature of the TreeNode constructor from:

    TreeNode(const string& name, const NodeParameters& params)

to:

    TreeNode(const string& name, const NodeConfiguration& config) 

where the definition of `NodeConfiguration` is:

```c++
typedef std::unordered_map<std::string, std::string> PortsRemapping;

struct NodeConfiguration
{
    Blackboard::Ptr blackboard;
    PortsRemapping  input_ports;
    PortsRemapping  output_ports;
};
```

# 4. Code example

Let's illustrate these changes with a practical example.

In this example __path__ is an output port in `ComputePath` but an input port
in `FollowPath`.

```XML
    <SequenceStar name="navigate">
        <Action ID="SaySomething" message="hello World"/>
        <Action ID="ComputePath" endpoints="{navigation_endpoints}" 
                                 path="{navigation_path}" />
        <Action ID="FollowPath"  path="{navigation_path}" />
    </SequenceStar>
```

No distinction is made in the XML between inputs, outputs;
additionally, passing static text parameters is __still__ possible 
(see "hello World" in SaySomething).

The actual entries to be read/written are the one specified in the remapping,
in this case:

 - when application code reads `endpoints`, it is actually reading `navigation_endpoints`.
 - when application code reads/writes `path`, it is actually accessing `navigation_path`. 

Since these names are specified in the XML, name clashing can be avoided without 
modifying the source code.

The corresponfing C++ code might be:

```C++

class SaySomething: public SyncActionNode
{
  public:
    SaySomething(const std::string& name, const NodeConfiguration& config): 
        SyncActionNode(name, config){}

    NodeStatus tick() override
    {
        auto msg = getInput<std::string>("message");
        if( !msg ) // msg is optional<std::string>
        { 
       	    return NodeStatus::FAILURE;
            // or...
            // throw if you think that this should not happen
            // or...
            // replace with default value 
        }
        std::cout << msg.value() << std::endl;
        return NodeStatus::SUCCESS;
    }
    static PortsList providedPorts()
    {
        static PortsList ports_list = { {"message", PortType::INPUT} );
        return ports_list;
    } 
};

class ComputePath: public SyncActionNode
{
  public:
    ComputePath(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name, config){}

    NodeStatus tick() override
    {
        auto end_points = getInput<EndPointsType>("endpoints");
        // do your stuff
        setOutput("path", my_computed_path);
        // return result...
    }
    static PortsList providedPorts()
    {
        static PortsList ports_list = { {"endpoints", PortType::INPUT}, 
                                        {"path",      PortType::OUTPUT} };
        return ports_list;
    } 
};

class FollowPath: public AsyncActionNode
{
  public:
    FollowPath(const std::string& name, const NodeConfiguration& config):
        AsyncActionNode(name, config){}

    NodeStatus tick() override
    {
        auto path = getInput<PathType>("path");
        // do your stuff
        // return result...
    }
    static PortsList providedPorts()
    {
        static PortsList ports_list = { {"path", PortType::INPUT} };
        return ports_list;
    } 
};
```

The user's code doesn't need to know if inputs where passed as "static text" 
or "blackboard pointers".





