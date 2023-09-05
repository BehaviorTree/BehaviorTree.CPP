#!/usr/bin/env python3

"""
Demonstration of an asynchronous action node implemented conveniently as a
Python coroutine. This enables simple synchronous code to be written in place of
complex asynchronous state machines.
"""

import time
import numpy as np
from btpy import (
    AsyncActionNode,
    BehaviorTreeFactory,
    SyncActionNode,
    NodeStatus,
    ports,
)


xml_text = """
 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
         <ReactiveSequence name="root">
             <Print value="{command}" />
             <MyAsyncNode start="[0.1, -0.2]" goal="[-0.6, 0.3]" command="{command}" />
         </ReactiveSequence>
     </BehaviorTree>

 </root>
"""


@ports(inputs=["start", "goal"], outputs=["command"])
class MyAsyncNode(AsyncActionNode):
    def run(self):
        start = np.asarray(self.get_input("start"))
        goal = np.asarray(self.get_input("goal"))

        # Here we write an imperative-looking loop, but we place a `yield` call
        # at each iteration. This causes the coroutine to yield back to the
        # caller until the next iteration of the tree, rather than block the
        # main thread.
        t0 = time.time()
        while (t := time.time() - t0) < 1.0:
            command = (1.0 - t) * start + t * goal
            self.set_output("command", command)
            yield

        print("Trajectory finished!")
        return NodeStatus.SUCCESS

    def on_halted(self):
        print("Trajectory halted!")


@ports(inputs=["value"])
class Print(SyncActionNode):
    def tick(self):
        value = self.get_input("value")
        if value is not None:
            print(value)
        return NodeStatus.SUCCESS


factory = BehaviorTreeFactory()
factory.register(MyAsyncNode)
factory.register(Print)

tree = factory.create_tree_from_text(xml_text)

# Run for a bit, then halt early.
for i in range(0, 10):
    tree.tick_once()
tree.halt_tree()
