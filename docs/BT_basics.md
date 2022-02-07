# Introduction to BTs

Unlike a Finite State Machine, a Behaviour Tree is a __tree of hierarchical nodes__ 
that controls the flow of execution of "tasks". 

## Basic Concepts

- A signal called "__tick__" is sent to the root of the tree
and propagates through the tree until it reaches a leaf node.

- A TreeNode that receives a __tick__ signal executes it's callback.
  This callback must return either

    - SUCCESS,
    - FAILURE or
    - RUNNING, if the action is asynchronous and it needs more time
      to complete.

- If a TreeNode has one or more children, it is in charge for ticking
  them, based on its state, external parameters or the result of the
  previous sibling.

 - The __LeafNodes__, those TreeNodes which don't have any children,
   are the actual commands, i.e. the place where the behavior tree
   interacts with the rest of the system.
   __Actions__ nodes are the most common type of LeafNodes.

!!! Note
    The word __tick__ will be often used as a *verb* (to tick / to be ticked) and it means
    
    "To invoke the callback `tick()` of a `TreeNode`".

In a service-oriented architecture, the leaves would contain
the "client" code that communicates with the "server",
that performs the actual operation.

## How tick works

To mentally visualize how ticking the tree works, consider the example below.

![basic sequence](images/bt_intro_01.gif)

A __Sequence__ is the simplest __ControlNode__: it execute 
its children one after the other and, if they all Succeed,
it returns SUCCESS (green) too.

1. The first tick set the Sequence node to RUNNING (orange).
2. Sequence tick the first child, "DetectObject", that eventually returns SUCCESS.
3. As a result, the second child "GraspObject" is ticked and the entire Sequence switch from RUNNING to SUCCESS.


## Types of nodes


![UML hierarchy](images/TypeHierarchy.png)

| Type of TreeNode  | Children Count     | Notes              |
| -----------       | ------------------ | ------------------ |
| ControlNode       | 1...N | Usually, ticks a child based on the result of its siblings or/and its own state.        |
| DecoratorNode     | 1     | Among other things, it may alter the result of the children or tick it multiple times.
| ConditionNode     | 0     | Should not alter the system. Shall not return RUNNING. |
| ActionNode        | 0     | It can alter the system.         |


In the context of __ActionNodes__, we may further distinguish between
synchronous and asynchronous nodes.

The former are executed atomically and block the tree until a SUCCESS or FAILURE is returned.

Asynchronous actions, instead, may return RUNNING to communicate that
the action is still being executed.

We need to tick them again, until SUCCESS or FAILURE is eventually returned.

# Examples

To better understand how BehaviorTrees work, let's focus on some practical
examples. For the sake of simplicity we will not take into account what happens when an action returns RUNNING.

We will assume that each Action is executed atomically and synchronously.


### First ControlNode: Sequence

Let's illustrate how a BT works using the most basic and frequently used 
ControlNode: the [SequenceNode](SequenceNode.md).

The children of a ControlNode are always __ordered__; in the graphical 
representation, the order of execution is __from left to right__.

![Simple Sequence: fridge](images/SequenceBasic.svg)


In short:

- If a child returns SUCCESS, tick the next one.
- If a child returns FAILURE, then no more children are ticked and the Sequence returns FAILURE.
- If __all__ the children return SUCCESS, then the Sequence returns SUCCESS too.

!!! warning "Have you spotted the bug?"
    If the action __GrabBeer__ fails, the door of the 
    fridge would remain open, since the last action __CloseFridge__ is skipped.


### Decorators

Depending on the type of [DecoratorNode](DecoratorNode.md), the goal of
this node could be either:

- to transform the result it received from the child.
- to halt the execution of the child.
- to repeat ticking the child, depending on the type of Decorator.


![Simple Decorator: Enter Room](images/DecoratorEnterRoom.svg)

The node __Inverter__ is a Decorator that inverts 
the result returned by its child; An Inverter followed by the node called
__isDoorOpen__ is therefore equivalent to 

    "Is the door closed?".

The node __Retry__ will repeat ticking the child up to __num_attempts__ times (5 in this case)
if the child returns FAILURE.

__Apparently__, the branch on the right side means: 

    If the door is closed, then try to open it.
    Try up to 5 times, otherwise give up and return FAILURE.
    
But...
    
!!! warning "Have you spotted the bug?"
    If __isDoorOpen__ returns FAILURE, we have the desired behaviour.
    But if it returns SUCCESS, the left branch fails and the entire Sequence
    is interrupted.
    
    We will see later how we can improve this tree. 
    

### Second ControlNode: Fallback

[FallbackNodes](FallbackNode.md), known also as __"Selectors"__,
are nodes that can express, as the name suggests, fallback strategies, 
i.e. what to do next if a child returns FAILURE.

It ticks the children in order and:

- If a child returns FAILURE, tick the next one.
- If a child returns SUCCESS, then no more children are ticked and the 
   Fallback returns SUCCESS.
- If all the children return FAILURE, then the Fallback returns FAILURE too.

In the next example, you can see how Sequences and Fallbacks can be combined:
    
![FallbackNodes](images/FallbackBasic.svg)  


> Is the door open?
>
> If not, try to open the door.
>
> Otherwise, if you have a key, unlock and open the door.
>
> Otherwise, smash the door. 
>
> If __any__ of these actions succeeded, then enter the room.

### "Fetch me a beer" revisited

We can now improve the "Fetch Me a Beer" example, which left the door open 
if the beer was not inside the fridge.

We use the color "green" to represent nodes which return
SUCCESS and "red" for those which return FAILURE. Black nodes haven't
been executed. 

![FetchBeer failure](images/FetchBeerFails.svg)

Let's create an alternative tree that closes the door even when __GrabBeer__ 
returns FAILURE.


![FetchBeer failure](images/FetchBeer.svg)

Both these trees will close the door of the fridge, eventually, but:

- the tree on the __left__ side will always return SUCCESS, no matter if
we have actually grabbed the beer.
 
- the tree on the __right__ side will return SUCCESS if the beer was there, 
FAILURE otherwise.

Everything works as expected if __GrabBeer__ returns SUCCESS.

![FetchBeer success](images/FetchBeer2.svg)



