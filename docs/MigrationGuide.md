# Migration Guide from V2 to V3

The main goal of this project is to create a Behavior Tree implementation
that uses the principles of Model Driven Development to separate the role 
of the __Component Developer__ from the __Behavior Designer__ and 
__System Integrator__.

In practice, this means that:

- Custom Actions (or, in general, custom TreeNodes) must be __reusable__ building
blocks. Implement them once, reuse them many times.

- To build a Behavior Tree out of TreeNodes, the Behavior Designer __must 
not need to read nor modify the source code__ of the a given TreeNode.

There was a major design flaw that undermined these goals in version  `2.x`: 
the way the BlackBoard was used to implement DataFlow between nodes.

As described in [issue #18](https://github.com/BehaviorTree/BehaviorTree.CPP/issues/18)
there are several potential problems with the Blackboard approach:

- To know which entries of the BB are read/written, you should read the source code.

- As a consequence, external tools such as __Groot__ can not know which BB entries are accessed.

- If there is a name clashing (multiple nodes use the same key for different purposes),
 the only way to fit it is modifying the source code. 

SMACH solved this problem using [input and output ports](http://wiki.ros.org/smach/Tutorials/User%20Data)
and remapping to connect them.

In the ROS community, we potentially have the same problem with topics,
but tools such as __rosinfo__ provides introspection at run-time and name
clashing is avoided using remapping.

This was the main reason to develop version `3.x` of __Behaviortree.CPP__, but we
also took the opportunity to do some additional refactoring to make the code
more understandable.

In this document we will use the following terms quite often:

- __Composition__: it refers to "composing" TreeNodes into Trees. In general
 we want a TreeNode implementation to be composition-agnostic.
 
- __Model/Modelling__: it is a description of a Tree or TreeNode that is 
sufficient (and necessary) to describe it, without knowing any additional 
detail about the actual C++ implementation.


## Blackboard, NodeParameters an DataPorts

In version `2.x` we had the intuition that passing one or more arguments
to a `TreeNode` would make the node more generic and reusable.

This is similar to the arguments of a function in any programming language.

```C++
// with arguments
GoTo("kitchen")

//Without arguments
GoToKitchen()
GoToLivingRoom()
GoToBedRoom1()
GoToBedroom2()
// ....
```

To pass NodeParameters we used the Blackboard, that is nothing more than a
shared __key/value__ table, i.e. a glorified bunch of global variables.

The key is a `string`, whilst the value is 
stored in a type-safe container similar to `std::any` or `std::variant`.

The problem is that writing/reading in an entry of the BB was done __implicitly__
in the source code and it was usually hard-coded. This made the TreeNode
not reusable.

To fix this, we still use the Blackboard under the hood, but it can not be 
accessed directly anymore. 

In version `3.x`Blackboard entries can be read/written using respectively 
`InputPorts` and `OutputPorts`.

These ports __must be defined explicitly__ to allow remapping at run-time.

Let's take a look to an example writte using the __old__ code:

```XML
<root>
     <BehaviorTree>
        <SequenceStar>
            <CalculateGoal/>
            <MoveBase  goal="${GoalPose}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
```

```C++
using namespace BT;
//Old code (V2)
NodeStatus CalculateGoal(TreeNode& self)
{
    const Pose2D mygoal = { 1, 2, 3.14};
    // "GoalPose" is hardcoded... we don't like that
    self.blackboard()->set("GoalPose", mygoal);
    return NodeStatus::SUCCESS;
}

class MoveBase : public AsyncActionNode
{
  public:

    MoveBase(const std::string& name, const NodeParameters& params)
      : AsyncActionNode(name, params) {}

    static const NodeParameters& requiredNodeParameters()
    {
        static NodeParameters params = {{"goal", "0;0;0"}};
        return params;
    }

    NodeStatus tick()
    {
        Pose2D goal;
        if (getParam<Pose2D>("goal", goal))
        {
            printf("[ MoveBase: DONE ]\n");
            return NodeStatus::SUCCESS;
        }
        else{
            printf("MoveBase: Failed for some reason\n");
            return NodeStatus::FAILURE;
        }
    }
    /// etc.
};
```

We may notice that the `NodeParameter` can be remapped in the XML, but
to change the key "GoalPose" in `CalculateGoalPose`we must inspect the code
and modify it.

In other words, `NodeParameter` is already a reasonably good implementation
of an `InputPort`, but we need to introduce a consistent `OutputPort` too.

This is the __new__ code:

```XML
<root>
     <BehaviorTree>
        <SequenceStar>
            <CalculateGoal target="{GoalPose}" />
            <MoveBase        goal="{GoalPose}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
```

```C++
using namespace BT;
//New code (V3)
class CalculateGoalPose : public SyncActionNode
{
public:

    MoveBase(const std::string& name, const NodeConfiguration& cfg)
      : SyncActionNode(name, cfg) {}

    static PortsList providedPorts()
    {
        return { OutputPort<Pose2D>("target") };
    }

    BT::NodeStatus tick()
    {
        const Pose2D myTarget = { 1, 2, 3.14 };
        setOutput("target", myTarget);
        return NodeStatus::SUCCESS;
    }
};

class MoveBase : public AsyncActionNode
{
public:

    MoveBase(const std::string& name, const NodeConfiguration& config)
      : AsyncActionNode(name, config) {}

    static PortsList providedPorts()
    {
        return { InputPort<Pose2D>("goal", "Port description", "0;0;0") };
    }

    NodeStatus tick()
    {
        Pose2D goal;
        if (auto res = getInput<Pose2D>("goal", goal))
        {
            printf("[ MoveBase: DONE ]\n");
            return NodeStatus::SUCCESS;
        }
        else{
            printf("MoveBase: Failed. Error code: %s\n", res.error());
            return NodeStatus::FAILURE;
        }
    }
    /// etc.
};
```

The main differences are:

- `requiredNodeParameters()` was replaced by `providedPorts()`, that
 is used to declare both Inputs and Output ports alike.
   
- `setOutput<>()` has been introduced. The method `blackboard()`can not be 
   accessed anymore.

- `getParam<>()` is now called `getInput<>()` to be more consistent with
  `setOutput<>()`. Furthermore, if an error occurs, we can get the error 
  message.
  
- Remapping to a shared entry ("GoalPose") is done at run-time in the XML.
  You will never need to modify the C++ source code.

## SubTrees, remapping and isolated Blackboards

Thanks to ports we solved the problem of __reusability of single treeNodes__.

But we still need to address the problem of __reusability of entire Trees/SubTrees__.

According to the rule of __hierarchical composition__,
from the point of view of a parent Node if should not matter if the 
child is a LeafNode, a DecoratorNode a ControlNode or an entire Tree.

As mentioned earlier, the Blackboard used to be a large key/value table.

Unfortunately, this might be challenging when we reuse multiple SubTree, once again
because of name clashing.

The solution in version `3.x` is to have a separated and isolated Blackboard
for each Tree/Subtree. If we want to connect the "internal" ports of a SubTree
with the other ports of the BB of the parent, we must explicitly do a 
remapping in the XML definition. No C++ code need to be modified.

From the point of view of the XML, remapped ports of a SubTree looks exactly
like the ports of a single node.

For more details, refer to the example __t06_subtree_port_remapping.cpp__.


## ControlNodes renamed/refactored

The [principle of least astonishment](https://en.wikipedia.org/wiki/Principle_of_least_astonishment)
applies to user interface and software design. A typical formulation of the principle, from 1984, is: 

>"If a necessary feature has a high astonishment factor, it may be necessary 
to redesign the feature.

In my opinion, the two main building blocks of BehaviorTree.CPP, the `SequenceNode` 
and the `FallbackNode` have a very high astonishment factor, because they are
__"reactive"__.

By "reactive" we mean that:

- Children (usually `ConditionNodes`) that returned 
  a valid value, such as SUCCESS or FAILURE, might be ticked again if another 
  child returns RUNNING.
  
- A different result in that Condition might abort/halt the RUNNING asynchronous child.


The main concern of the original author of this library was to build reactive
Behavior Trees (see for reference this [publication](https://arxiv.org/abs/1709.00084)).

I share this goal, but I prefer to have more explicit names, because reactive 
ControlNodes are useful but hard to reason about sometimes.

I don't think reactive ControlNodes should be used mindlessly by default. 

For instance, most of the time users I talked with should have used `SequenceStar`
instead of `Sequence` in many cases.

I renamed the ControlNodes as follows to reflect this reality:


| Old Name (v2)  |  New name (v3) | Is reactive?  |
|:--- |:--- |:---:|
| Sequence | ReactiveSequence  | YES  |
| SequenceStar (reset_on_failure=true)  |  Sequence |  NO |
| SequenceStar (reset_on_failure=false) |  SequenceStar |  NO |
| Fallback | ReactiveFallback  | YES  |
| FallbackStar  |  Fallback |  NO |
| Parallel |  Parallel |  Yes(v2) / No(v3) |


A reactive `ParallelNode` was very confusing and error prone; in most cases, 
what you really want is you want to use a `ReactiveSequence` instead.

In version `2.x` it was unclear what would happen if a "reactive" node has
more than a single asynchronous child. 

The new recommendation is: 

>__Reactive nodes should NOT have more than a single asynchronous child__.

This is a very opinionated decision and for this reason it is documented but 
not enforced by the implementation.







 






