# Roadmap: input/output ports in TreeNode

## Introduction to the problem

One of the goals of this project is to separate the role of the Component
Developer from the Behavior Designed and System Integrator.

As a consequence, in the contect of BehaviorTree we want to write custom
ActionNodes and ConditionNodes once and never touch that source code.

Using the same precompiled nodes, it should be possible to build any tree.

We realized that there is a major desgn flow that undermine this goal: the way
dataflow between nodes is done using the BlackBoard.


As described in [issue #18](https://github.com/BehaviorTree/BehaviorTree.CPP/issues/18)
there are several issues:

- To know which entries of the BB are read/written, you should inspect the source code.
- As a consequence, external tools such as __Groot__ have no idea of which BB entries are accessed.
- If there is a name clashing (multiple nodes use the same key for different purposes),
 the only way to solve it is modifying the source code. 

SMACH solved this problem using [input and output ports](http://wiki.ros.org/smach/Tutorials/User%20Data)
and remapping to connect them.

## Suggested changes

Goals of the new design:

- The [TreeNodeManifest](https://github.com/BehaviorTree/BehaviorTree.CPP/blob/master/include/behaviortree_cpp/bt_factory.h#L33)
should contain information about input and outputs ports, to make this information available
to external tools.

- Avoid name clashing using key remapping.

- We want to solve the previous problems but trying to keep the API as consistent
as possible with the previous one.

### Deprecate TreeNode::blackboard()

Accessing directly the BB allows the users to do whatever they wants.
There is no way to introspect which entries are accessed.

Therefore, the only reasonable thing to do is to deprecate `TreeNode::blackboard()`

The problem is that `SimpleActionNodes` and `SimpleDecoratorNodes` 
will loose the ability to access ports.

### NameParameters as input ports

We know that NodeParameters are a mechanism to add "arguments" to a Node.

It is possible to point to the entry of the BB, instead of parsing a static value.
After few months, it became clear that this is the rule rather thatn the exception.

In probably 80-90% of the cases, NodeParameters are passed through the BB.

Furthermore, `requiredNodeParameters` is already an effective way to 
automatically create a manifest.

As a consequence, we may consider them already a valid implementation of an
__input port__.

From a practical point of view, the user should encourage the user to use
`TreeNode::getParam` as much as possible and deprecate `TreeNode::blackboard()::get`

### Output Ports

We need to add automatically the output ports to the TreeNodeManifest.

To do that, we can just add the static method

       const std::set<std::string>& providedOutputPorts()
      
for consistency, we might consider to change the signature of  `requiredNodeParameters()` to
   
       const std::set<std::string>& requiredNodeParameters()

In other words, requiredNodeParameters provides only the key, but not a default value;
in fact, we have seen that there is little practical use for a default value.

The new manifest definition could be:

```c++
struct TreeNodeManifest
{
    NodeType type;
    std::string registration_ID;
    std::set<std::string> required_parameters;
    std::set<std::string> provided_outputs;
};
```

About remapping, to avoid name cashing it is sufficient to provide remapping
at run-time __only for the output ports__.

We don't need remapping of input ports, because the name of the entry is 
already provided at run-time (in the XML).

Example:

__TODO__

### Major (breaking) changes in the signature of TreeNodes

__Under development...__

Does it make sense to change the signature of the TreeNode constructor from:

    TreeNode(const string& name, const NodeParameters& params)

to:

    TreeNode(const string& name, const NodeConfiguration& config) 

where:

```c++
struct NodeConfiguration
{
    // needed to register this in the constructor
    BlackBoard::Ptr blackboard;
    
    // needed to register this in the constructor 
    std::string registration_ID;
    
    // input parameters. Might be strings or pointers to BB entries
    NodeParameters parameters;
    
    // Provide simulatenously a list of output ports and
    // their remapped keys.
    std::unordered_map<std::string, std::string> remapped_outputs;
};
```

This would solve multiple problems, including:

- The fact that BB are not available in the constructor.
- Potential errors when `setRegistrationName()` in not called.
- provides remapping.  
