# Fallback

This family of nodes are known as "Selector" or, sometimes, "Priority"
in other frameworks.

Its purpose is to try different strategies, until we find one that "works".
Currently, there is only a single type of node called "FallbackNode".


## FallbackNode

The SequenceNode is used to execute the children in a sequence.

- Before ticking the first child, Fallback becomes __RUNNING__.
- If a child returns __FAILURE__, it ticks the next child.
- If the __last__ child returns __FAILURE__ too, all the children are halted and
 the Sequence returns __FAILURE__.
- If a child returns __RUNNING__, Fallback suspends and returns __RUNNING__.
- If a child returns __SUCCESS__, Fallback stops and returns __SUCCESS__. 

__Example__:

Try different strategies to open the door. Check first if it is open already.

![FallbackNode](images/FallbackSimplified.png)

??? example "See the pseudocode"
	``` c++
		// At the beginning, start from first child 
		if( state != RUNNING) {
			index = 0;
		}
		state = RUNNING;

		while( index < number_of_children )
		{
			child_state = child[index]->tick();
			
			if( child_state == RUNNING ) {
				// Suspend execution and return RUNNING.
				// At the next tick, index will be the same.
				state = RUNNING;
				return state;
			}
			else if( child_state == FAILURE ) {
				// continue the while loop
				index++;
			}
			else if( child_state == SUCCESS ) {
				// Suspend execution and return SUCCESS.
				// index is reset and children are halted.
				state = SUCCESS;
				index = 0;
				HaltAllChildren();
				return state;
			}
		}
		// all the children returned failure. Return FAILURE too.
		state = FAILURE;
		HaltAllChildren();
		return state;




 
