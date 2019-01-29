# Remapping of ports between SubTrees and parent Tree 

In the CrossDoor example we saw that a `SubTree` looks like a single
leaf Node from the point of view of its parent (`MainTree` in the example).

Furthermore, to avoid name clashing in very large trees, any tree and subtree
use a different instance of the Blackboard.

For this reason, we need to explicitly connect the ports of a tree to those
of its subtrees.

Once again, you will not need to modify your C++ implementation since this 
remapping is done in the XML definition.
