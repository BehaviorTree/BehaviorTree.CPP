# Composition of Behaviors with Subtree

We can build large scale behavior composing together smaller and reusable
behaviors into larger ones.

In other words, we want to create __hierarchical__ behavior trees. 

This can be achieved easily defining multiple trees in the XML including one
into the other.

# CrossDoor behavior

This example is inspired by a popular 
[article about behavior trees](http://www.gamasutra.com/blogs/ChrisSimpson/20140717/221339/Behavior_trees_for_AI_How_they_work.php).

It is also the first practical example that uses `Decorators` and `Fallback`.

```XML hl_lines="1 3 15"
<root main_tree_to_execute = "MainTree">
	
    <BehaviorTree ID="DoorClosed">
        <Sequence name="door_closed_sequence">
            <Inverter>
                <IsDoorOpen/>
            </Inverter>
            <RetryUntilSuccesful num_attempts="4">
                <OpenDoor/>
            </RetryUntilSuccesful>
            <PassThroughDoor/>
        </Sequence>
    </BehaviorTree>
    
    <BehaviorTree ID="MainTree">
        <Fallback name="root_Fallback">
            <Sequence name="door_open_sequence">
                <IsDoorOpen/>
                <PassThroughDoor/>
            </Sequence>
            <SubTree ID="DoorClosed"/>
            <PassThroughWindow/>
        </Fallback>
    </BehaviorTree>
    
</root>
```

It may be noticed that we incapsulated a quite complex branch of the tree,
the one to execute when the door is closed, into a separate tree called
`DoorClosed`.

The desired behavior is:

- If the door is open, `PassThroughDoor`.
- If the door is closed, try up to 4 times to `OpenDoor` and, then, `PassThroughDoor`.
- If it was not possible to open the closed door, `PassThroughWindow`.


## Loggers

On the C++ side we don't need to do anything to build reusable subtrees.

Therefore we take this opportunity to introduce another neat feature of
_BehaviorTree.CPP_ : __Loggers__.

A Logger is a mechanism to display, record and/or publish any state change in the tree.


```C++

int main()
{
    using namespace BT;
    BehaviorTreeFactory factory;

    // register all the actions into the factory
    // We don't show how these actions are implemented, since most of the 
    // times they just print a message on screen and return SUCCESS.
    // See the code on Github for more details.
    factory.registerSimpleCondition("IsDoorOpen", std::bind(IsDoorOpen));
    factory.registerSimpleAction("PassThroughDoor", std::bind(PassThroughDoor));
    factory.registerSimpleAction("PassThroughWindow", std::bind(PassThroughWindow));
    factory.registerSimpleAction("OpenDoor", std::bind(OpenDoor));
    factory.registerSimpleAction("CloseDoor", std::bind(CloseDoor));
    factory.registerSimpleCondition("IsDoorLocked", std::bind(IsDoorLocked));
    factory.registerSimpleAction("UnlockDoor", std::bind(UnlockDoor));

    // Load from text or file...
    auto tree = factory.createTreeFromText(xml_text);

    // This logger prints state changes on console
    StdCoutLogger logger_cout(tree.root_node);

    // This logger saves state changes on file
    FileLogger logger_file(tree.root_node, "bt_trace.fbl");
    
    // This logger stores the execution time of each node
    MinitraceLogger logger_minitrace(tree.root_node, "bt_trace.json");

    printTreeRecursively(tree.root_node);

    //while (1)
    {
        NodeStatus status = NodeStatus::RUNNING;
        // Keep on ticking until you get either a SUCCESS or FAILURE state
        while( status == NodeStatus::RUNNING)
        {
            status = tree.root_node->executeTick();
            CrossDoor::SleepMS(1);   // optional sleep to avoid "busy loops"
        }
        CrossDoor::SleepMS(2000);
    }
    return 0;
}

```




