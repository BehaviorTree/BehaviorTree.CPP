# How to use multiple XML files to store your subtrees

In the examples which we presented so far, we always create an entire Tree
from a single XML file.

If multiple Subtrees were used, they were all included into the same XML.

In recent version of BT.CPP (3.7+), the user can more easily
load trees from multiple files, if needed.

## Load multiple files with "include"

We will consider a main tree that invoke 2 different subtrees.

File **main_tree.xml**:

```XML hl_lines="2 3"
<root main_tree_to_execute = "MainTree">
    <include path="./subtree_A.xml" />
    <include path="./subtree_B.xml" />
    <BehaviorTree ID="MainTree">
        <Sequence>
            <SaySomething message="starting MainTree" />
            <SubTree ID="SubTreeA" />
            <SubTree ID="SubTreeB" />
        </Sequence>
    </BehaviorTree>
<root>
```
File **subtree_A.xml**:

```XML
<root>
    <BehaviorTree ID="SubTreeA">
        <SaySomething message="Executing Sub_A" />
    </BehaviorTree>
</root>
```

File **subtree_B.xml**:

```XML
<root>
    <BehaviorTree ID="SubTreeB">
        <SaySomething message="Executing Sub_B" />
    </BehaviorTree>
</root>
```

As you may notice, we included two relative paths in **main_tree.xml**
that tells to `BehaviorTreeFactory` where to find the required dependencies.

We need to create the tree as usual:

```c++
factory.createTreeFromFile("main_tree.xml")
```

## Load multiple files manually

If we don't want to add relative and hard-coded paths into our XML,
or if we want to instantiate a subtree instead of the main tree, there is a
new approach, since BT.CPP 3.7+.

The simplified version of **main_tree.xml** will be:

```XML
<root>
    <BehaviorTree ID="MainTree">
        <Sequence>
            <SaySomething message="starting MainTree" />
            <SubTree ID="SubTreeA" />
            <SubTree ID="SubTreeB" />
        </Sequence>
    </BehaviorTree>
<root>
```

To load manually the multiple files:

```c++
int main()
{
    BT::BehaviorTreeFactory factory;
    factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

    // Register the behavior tree definitions, but don't instantiate them, yet.
    // Order is not important.
    factory.registerBehaviorTreeFromFile("main_tree.xml");
    factory.registerBehaviorTreeFromFile("subtree_A.xml");
    factory.registerBehaviorTreeFromFile("subtree_B.xml");

    //Check that the BTs have been registered correctly
    std::cout << "Registered BehaviorTrees:" << std::endl;
    for(const std::string& bt_name: factory.registeredBehaviorTrees())
    {
        std::cout << " - " << bt_name << std::endl;
    }

    // You can create the MainTree and the subtrees will be added automatically.
    std::cout << "----- MainTree tick ----" << std::endl;
    auto main_tree = factory.createTree("MainTree");
    main_tree.tickRoot();

    // ... or you can create only one of the subtree
    std::cout << "----- SubA tick ----" << std::endl;
    auto subA_tree = factory.createTree("SubTreeA");
    subA_tree.tickRoot();

    return 0;
}
/* Expected output:

Registered BehaviorTrees:
 - MainTree
 - SubTreeA
 - SubTreeB
----- MainTree tick ----
Robot says: starting MainTree
Robot says: Executing Sub_A
Robot says: Executing Sub_B
----- SubA tick ----
Robot says: Executing Sub_A
```




