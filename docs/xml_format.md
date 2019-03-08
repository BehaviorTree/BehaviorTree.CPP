
## Basics of the XML schema

In the [first tutorial](tutorial_01_first_tree.md) this simple tree
was presented.

``` XML
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
            <SaySomething   name="action_hello" message="Hello"/>
            <OpenGripper    name="open_gripper"/>
            <ApproachObject name="approach_object"/>
            <CloseGripper   name="close_gripper"/>
        </Sequence>
     </BehaviorTree>
 </root>
```

You may notice that:

- The first tag of the tree is `<root>`. It should contain __1 or more__ tags `<BehaviorTree>`.

- The tag `<BehaviorTree>` should have the attribute `[ID]`.

- The tag `<root>` should contain the attribute `[main_tree_to_execute]`.

- The attribute `[main_tree_to_execute]` is mandatory if the file contains multiple `<BehaviorTree>`, 
  optional otherwise.

- Each TreeNode is represented by a single tag. In particular:

     - The name of the tag is the __ID__ used to register the TreeNode in the factory.
     - The attribute `[name]` refers to the name of the instance and is __optional__.
     - Ports are configured using attributes. In the previous example, the action 
     `SaySomething` requires the input port `message`.

- In terms of number of children:

     - `ControlNodes` contain __1 to N children__.
     - `DecoratorNodes` and Subtrees contain __only 1 child__.
     - `ActionNodes` and `ConditionNodes` have __no child__. 

## Ports Remapping and pointers to Blackboards entries

As explained in the [second tutorial](tutorial_02_basic_ports.md)
input/output ports can be remapped using the name of an entry in the
Blackboard, in other words, the __key__ of a __key/value__ pair of the BB.

An BB key is represented using this syntax: `{key_name}`.

In the following example:

- the first child of the Sequence prints "Hello",
- the second child reads and writes the value contained in the entry of 
  the blackboard called "my_message"; 

``` XML
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
            <SaySomething message="Hello"/>
            <SaySomething message="{my_message}"/>
        </Sequence>
     </BehaviorTree>
 </root>
```
     

## Compact vs Explicit representation

The following two syntaxes are both valid:

``` XML
 <SaySomething               name="action_hello" message="Hello World"/>
 <Action ID="SaySomething"   name="action_hello" message="Hello World"/>
```

We will call the former syntax "__compact__" and the latter "__explicit__".
The first example represented with the explicit syntax would become:

``` XML
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
           <Action ID="SaySomething"   name="action_hello" message="Hello"/>
           <Action ID="OpenGripper"    name="open_gripper"/>
           <Action ID="ApproachObject" name="approach_object"/>
           <Action ID="CloseGripper"   name="close_gripper"/>
        </Sequence>
     </BehaviorTree>
 </root>
```

Even if the compact syntax is more convenient and easier to write, it provides 
too little information about the model of the TreeNode. Tools like __Groot__ require either
the _explicit_ syntax or additional information.
This information can be added using the tag `<TreeNodeModel>`.

To make the compact version of our tree compatible with Groot, the XML 
must be modified as follows:


``` XML
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
           <SaySomething   name="action_hello" message="Hello"/>
           <OpenGripper    name="open_gripper"/>
           <ApproachObject name="approach_object"/>
           <CloseGripper   name="close_gripper"/>
        </Sequence>
    </BehaviorTree>
	
	<!-- the BT executor don't require this, but Groot does --> 	
    <TreeNodeModel>
        <Action ID="SaySomething">
            <input_port name="message" type="std::string" />
        </Action>
        <Action ID="OpenGripper"/>
        <Action ID="ApproachObject"/>
        <Action ID="CloseGripper"/>      
    </TreeNodeModel>
 </root>
```

!!! Note "XML Schema available for explicit version"
    You can download the [XML Schema](https://www.w3schools.com/xml/schema_intro.asp) here:
    [behaviortree_schema.xsd](https://github.com/BehaviorTree/BehaviorTree.CPP/blob/master/behaviortree_schema.xsd).

## Subtrees

As we saw in [this tutorial](tutorial_06_subtree_ports.md), it is possible to include
a Subtree inside another tree to avoid "copy and pasting" the same tree in
multiple location and to reduce complexity.

Let's say that we want to incapsulate few action into the behaviorTree "__GraspObject__" 
(being optional, attributes [name] are omitted for simplicity).

``` XML  hl_lines="6"
 <root main_tree_to_execute = "MainTree" >
 
     <BehaviorTree ID="MainTree">
        <Sequence>
           <Action  ID="SaySomething"  message="Hello World"/>
           <Subtree ID="GraspObject"/>
        </Sequence>
     </BehaviorTree>
     
     <BehaviorTree ID="GraspObject">
        <Sequence>
           <Action ID="OpenGripper"/>
           <Action ID="ApproachObject"/>
           <Action ID="CloseGripper"/>
        </Sequence>
     </BehaviorTree>  
 </root>
```

We may notice as the entire tree "GraspObject" is executed after "SaySomething".

## Include external files

__Since version 2.4__.

You can include external files in a way that is similar to __#include <file>__ in C++.
We can do this easily using the tag:

``` XML
  <include path="relative_or_absolute_path_to_file">
``` 

using the previous example, we may split the two behavior trees into two files:


``` XML hl_lines="5"
 <!-- file maintree.xml -->

 <root main_tree_to_execute = "MainTree" >
	 
	 <include path="grasp.xml"/>
	 
     <BehaviorTree ID="MainTree">
        <Sequence>
           <Action  ID="SaySomething"  message="Hello World"/>
           <Subtree ID="GraspObject"/>
        </Sequence>
     </BehaviorTree>
  </root>
``` 

``` XML
 <!-- file grasp.xml -->

 <root main_tree_to_execute = "GraspObject" >
     <BehaviorTree ID="GraspObject">
        <Sequence>
           <Action ID="OpenGripper"/>
           <Action ID="ApproachObject"/>
           <Action ID="CloseGripper"/>
        </Sequence>
     </BehaviorTree>  
 </root>
```

!!! Note "Note for ROS users"
    If you want to find a file inside a [ROS package](http://wiki.ros.org/Packages), 
    you can use this syntax:
    
    `<include ros_pkg="name_package"  path="path_relative_to_pkg/grasp.xml"/>`



 





       

