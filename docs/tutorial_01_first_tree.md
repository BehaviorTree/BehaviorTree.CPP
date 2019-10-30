# How to create a BehaviorTree

Behavior Trees, similar to State Machines, are nothing more than a mechanism
to invoke __callbacks__ at the right time under the right conditions.

Further, we will use the words __"callback"__ and __"tick"__ interchangeably.

What happens inside these callbacks is up to you.

In this tutorial series, most of the time Actions will just print some 
information on console,
but keep in mind that real "production" code would probably do something
more complicated.

## How to create your own ActionNodes

The default (and recommended) way to create a TreeNode is by inheritance.

``` c++
// Example of custom SyncActionNode (synchronous action)
// without ports.
class ApproachObject : public BT::SyncActionNode
{
  public:
    ApproachObject(const std::string& name) :
        BT::SyncActionNode(name, {})
    {
    }

    // You must override the virtual function tick()
    BT::NodeStatus tick() override
    {
        std::cout << "ApproachObject: " << this->name() << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
};
``` 

As you can see:

- Any instance of a TreeNode has a `name`. This identifier is meant to be 
  human-readable and it __doesn't__ need to be unique.
 
- The method __tick()__ is the place where the actual Action takes place.
  It must always return a NodeStatus, i.e. RUNNING, SUCCESS or FAILURE. 

Alternatively, we can use __dependecy injection__ to create a TreeNode given 
a function pointer (i.e. "functor"). 

The only requirement of the functor is to have either one of these signatures:

``` c++
    BT::NodeStatus myFunction()
    BT::NodeStatus myFunction(BT::TreeNode& self) 
```


For example:


``` c++
using namespace BT;

// Simple function that return a NodeStatus
BT::NodeStatus CheckBattery()
{
    std::cout << "[ Battery: OK ]" << std::endl;
    return BT::NodeStatus::SUCCESS;
}

// We want to wrap into an ActionNode the methods open() and close()
class GripperInterface
{
public:
    GripperInterface(): _open(true) {}
    
	NodeStatus open() {
		_open = true;
		std::cout << "GripperInterface::open" << std::endl;
		return NodeStatus::SUCCESS;
	}

	NodeStatus close() {
		std::cout << "GripperInterface::close" << std::endl;
		_open = false;
		return NodeStatus::SUCCESS;
	}

private:
    bool _open; // shared information
};

``` 

We can build a `SimpleActionNode` from any of these functors:

- CheckBattery()
- GripperInterface::open()
- GripperInterface::close()

## Create a tree dynamically with an XML

Let's consider the following XML file named __my_tree.xml__:


``` XML
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
            <SayHello       name="action_hello"/>
            <OpenGripper    name="open_gripper"/>
            <ApproachObject name="approach_object"/>
            <CloseGripper   name="close_gripper"/>
        </Sequence>
     </BehaviorTree>
 </root>
```

!!! Note
    You can find more details about the XML schema [here](xml_format.md).


We must first register our custom TreeNodes into the `BehaviorTreeFactory`
 and then load the XML from file or text.

The identifier used in the XML must coincide with those used to register
the TreeNodes.

The attribute "name" represents the name of the instance; it is optional.


``` c++
#include "behaviortree_cpp_v3/bt_factory.h"

// file that contains the custom nodes definitions
#include "dummy_nodes.h"

int main()
{
    // We use the BehaviorTreeFactory to register our custom nodes
    BehaviorTreeFactory factory;

    // Note: the name used to register should be the same used in the XML.
    using namespace DummyNodes;

    // The recommended way to create a Node is through inheritance.
    factory.registerNodeType<ApproachObject>("ApproachObject");

    // Registering a SimpleActionNode using a function pointer.
    // you may also use C++11 lambdas instead of std::bind
    factory.registerSimpleCondition("CheckBattery", std::bind(CheckBattery));

    //You can also create SimpleActionNodes using methods of a class
    GripperInterface gripper;
    factory.registerSimpleAction("OpenGripper", 
                                 std::bind(&GripperInterface::open, &gripper));
    factory.registerSimpleAction("CloseGripper", 
                                 std::bind(&GripperInterface::close, &gripper));

    // Trees are created at deployment-time (i.e. at run-time, but only 
    // once at the beginning). 
    
    // IMPORTANT: when the object "tree" goes out of scope, all the 
    // TreeNodes are destroyed
    auto tree = factory.createTreeFromFile("./my_tree.xml");

    // To "execute" a Tree you need to "tick" it.
    // The tick is propagated to the children based on the logic of the tree.
    // In this case, the entire sequence is executed, because all the children
    // of the Sequence return SUCCESS.
    tree.root_node->executeTick();

    return 0;
}

/* Expected output:
*
       [ Battery: OK ]
       GripperInterface::open
       ApproachObject: approach_object
       GripperInterface::close
*/

``` 



