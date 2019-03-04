# Summary of the tutorials

### T.1: Create your first Behavior Tree

This tutorial demonstrates how to create custom `ActionNodes` in __C++__ and 
how to compose them into Trees using the __XML__ language.

### T.2: Parametrize a Node with Ports

TreeNodes can have both Inputs and Outputs Ports.
This tutorial demonstrates how to use ports to create parametrized Nodes.


### T.3: Generic and type-safe Ports

This tutorial is an extension of the previous one.

It shows how to create and use ports with generic and user-defined
types.

### T.4: Difference between Sequence and ReactiveSequence

Reactive ControlNodes can be a very powerful tool to create sophisticated
behaviors.

This example shows the difference between a standard Sequence and a Reactive one.

### T.5: How to reuse entire SubTrees

Reusability and Composability can be done at the level of a single Node,
but also with entire Trees, which can become SubTrees of a "parent" Tree.

In this tutorial we will also introduce the builtin Loggers.

### T.6: Remapping of Ports between SubTrees and their parents

Any Tree/SubTree in the system has its own isolated BlackBoard.

In this tutorial we extend the concept or Ports to SubTrees, using 
port remapping.

### T.7: How to wrap legacy code in a non intrusive way

This tutorial shows one of the many possible ways to wrap an existing code
into the `BehavioTree.CPP` infrastructure.

### T.8: Passing arguments to Nodes without Ports

If your custom Node has a lot of ports, it is probably a sign that you didn't 
understand the problem that Ports are supposed to solve ;)

In this tutorial, we show how to pass arguments to a custom Node class without 
polluting your interfaces with pointless Input Ports.

### T.9: Asynchronous actions with Coroutines

Coroutines are a powerful tool to create asynchronous code.

In this tutorial, we outline the typical design pattern to use when you 
implement an asynchronous Action using `CoroActionNode`.




