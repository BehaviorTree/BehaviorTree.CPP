# ControlNodes

ControlNodes can have multiple children. Children are always __ordered__
and it is up to the ControlNode itself to decide if and when a child should
be ticked.

## SequenceNode

The SequenceNode is used to execute the children in a sequence.

It ticks its children __as long as__ they returns SUCCESS.

- Before ticking the first child, Sequence becomes __RUNNING__.
- If a child return __SUCCESS__, it ticks the next child.
- If the __last__ child returns __SUCCESS__ too, all the children are halted and
 the Sequence returns __SUCCESS__.
- If a child returns __RUNNING__, Sequence suspends and returns __RUNNING__.
- If a child returns __FAILURE__, Sequence stops and returns __FAILURE__. 







 
