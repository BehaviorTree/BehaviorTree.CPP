# Getting started

__BehaviorTree.CPP__ is a C++ library that can be easily integrated into
your favourite distributed middleware, such as __ROS__ or __SmartSoft__.

You can statically link it into your application (for example a game).

These are the main concepts which you need to understand first.

## Nodes vs Trees

The user must create his/her own ActionNodes and ConditionNodes (LeafNodes);
this library helps you to compose them easily into trees. 

Think about the LeafNodes as the building blocks which you need to compose
a complex system.

By definition, your custom Nodes are (or should be) highly __reusable__.
But, at the beginning, some wrapping interfaces might be needed to
adapt your legacy code.


## The tick() callbacks

Any TreeNode can be seen as a mechanism to invoke a __callback__, i.e. to 
__run a piece of code__. What this callback does is up to you.

In most of the following tutorials, our Actions will simply
print messages on console or sleep for a certain amount of time to simulate
a long calculation.

In production code, especially in Model Driven Development and Component 
Based Software Engineering, an Action/Condition would probably communicate
to other _components_ or _services_ of the system.

## Inheritance vs dependency injection.

To create a custom TreeNode, you should inherit from the proper class.

For instance, to create your own synchronous Action, you should inherit from the 
class __SyncActionNode__.

Alternatively, the library provides a mechanism to create a TreeNode passing a 
__function pointer__ to a wrapper (dependency injection).

This approach reduces the amount of boilerplate in your code; as a reference
please look at the [first tutorial](tutorial_01_first_tree.md) and the one
describing [non intrusive integration with legacy code](tutorial_07_legacy.md).

## Dataflow, Ports and Blackboard

Ports are explained in detail in the [second](tutorial_02_basic_ports.md)
and [third](tutorial_03_generic_ports.md) tutorials.

For the time being, it is important to know that:

- A __Blackboard__ is a _key/value_ storage shared by all the Nodes of a Tree.

- __Ports__ are a mechanism that Nodes can use to exchange information between
  each other.
  
- Ports are _"connected"_ using the same _key_ of the blackboard.

- The number, name and kind of ports of a Node must be known at _compilation-time_ (C++); 
  connections between ports are done at _deployment-time_ (XML).  


## Load trees at run-time using the XML format

Despite the fact that the library is written in C++, trees themselves
can be composed at _run-time_, more specifically, at _deployment-time_, since
it is done only once at the beginning to instantiate the Tree.

An XML format is described in details [here](xml_format.md).



