# Migration Guide from V2 to V3

The main goal of this project id to create a Behavior Tree implementation
that uses the principle of Model Driven Development to separate the role 
of the __Component Developer__ from the __Behavior Designed__ and __System Integrator__.

In practice, this means that:

- Custom Actions (or, in general, custom TreeNodes) must be reusable building
blocks. Implement them once, reuse them many times.

- To build a BehaviorTree out of TreeNodes, the Behavior Designer must not need to read 
nor modify the source code of the a given TreeNode.

There is a __major design flaw__ that undermines this goal: the way
the BlackBoard was used in version `2.x` to implement dataflow between nodes.

In general, DataFlow should not be the main concern of a library like this,
that focuses on Coordination, but it is apparent to anyone that had implemented 
a sufficiently large coordination component that if would be impossible 
to ignore it completely.

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

In this document we will use the following terms often:

- __Composition__: it refers to "composing" TreeNodes into Trees. In general
 we want a TreeNode implementation to be composition-agnostic.
 
- __Model/Modelling__: it is a description of a Tree or TreeNode that is 
sufficient (and necessary) to describe it, without knowing any additional 
detail about the actual implementation.


# 2. Blackboard, NodeParameters an DataPorts

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

On the other hand, we had the Blackboard, that was nothing more than a
shared __key/value__ table, i.e. a glorified bunch of glabal variables.

The key is a `string`, whilst the value is 
stored in a type-safe container similar to `std::any` or `std::variant`.

The problem is that writing/reading in an entry of the BB is done __implicitly__
in the source code and it is usually hard-coded. This makes the TreeNode
not reusable.

To fix this, we still use the Blackboard under the hood, but it can not be 
accessed directly anymore. Entries are read/written using respectively `InputPorts`
and `OutputPorts`.

These ports __must be modelled__ to allow remapping at run-time.

Let's take a look to an example at the old code:

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
//Old code (V2)
NodeStatus CalculateGoal(TreeNode& self)
{
    const Pose2D mygoal = { 1, 2, 3.14};
    // "GoalPose" is hardcoded... we don't like that
    self.blackboard()->set("GoalPose", mygoal);
    return NodeStatus::SUCCESS;
}

class MoveBase : public BT::AsyncActionNode
{
  public:

    MoveBase(const std::string& name, const BT::NodeParameters& params)
      : AsyncActionNode(name, params) {}

    static const BT::NodeParameters& requiredNodeParameters()
    {
        static BT::NodeParameters params = {{"goal", "0;0;0"}};
        return params;
    }

    BT::NodeStatus tick()
    {
        Pose2D goal;
        if (getParam<Pose2D>("goal", goal))
        {
            printf("[ MoveBase: DONE ]\n");
            return BT::NodeStatus::SUCCESS;
        }
        else{
            printf("MoveBase: Failed for some reason\n");
            return BT::NodeStatus::FAILURE;
        }
    }
    /// etc.
};
```

We may notice that the `NodeParameter` can be remapped in the XML, but
to change the key "GoalPose" in `CalculateGoalPose`we must inspect the code
and modify it.

In other words, `NodeParameter` is already a reasonably good implementation
of an `InputPort`, but we need to intoruce a consistent `OutputPort` too.

This is the new code:

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
//New code (V3)
class CalculateGoalPose : public BT::SyncActionNode
{
public:

    MoveBase(const std::string& name, const BT::NodeConfiguration& cfg)
      : SyncActionNode(name, cfg) {}

    static BT::PortsList providedPorts()
    {
        return { BT::OutputPort<Pose2D>("target") };
    }

    BT::NodeStatus tick()
    {
        const Pose2D myTarget = { 1, 2, 3.14 };
        setOutput("target", myTarget);
        return BT::NodeStatus::SUCCESS;
    }
};

class MoveBase : public BT::AsyncActionNode
{
public:

    MoveBase(const std::string& name, const BT::NodeConfiguration& config)
      : AsyncActionNode(name, config) {}

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<Pose2D>("goal", "Port description", "0;0;0") };
    }

    BT::NodeStatus tick()
    {
        Pose2D goal;
        if (auto res = getInput<Pose2D>("goal", goal))
        {
            printf("[ MoveBase: DONE ]\n");
            return BT::NodeStatus::SUCCESS;
        }
        else{
            printf("MoveBase: Failed. Error code: %s\n", res.error());
            return BT::NodeStatus::FAILURE;
        }
    }
    /// etc.
};
```

The main differences are:

- `requiredNodeParameters()` was replaced by `providedPorts()`, that
 it is used to declare both Inputs and Output ports alike.
   
- `setOutput<>()` has been introduced. The method `blackboard()`can not be 
   accessed anymore.

- `getParam<>()` is now called `getInput<>()` to be more consistent with
  `setOutput<>()`. Furthermore, if an error occurs, we can get the error 
  message.
  
- Remapping to a shared entry ("GoalPose") is done at run-time in the XML.

