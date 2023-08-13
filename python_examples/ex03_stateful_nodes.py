#!/usr/bin/env python3

"""
Demonstration of stateful action nodes.

To run, ensure that the `btpy_cpp` Python extension is on your `PYTHONPATH`
variable. It is probably located in your build directory if you're building from
source.
"""

import numpy as np
from btpy import (
    BehaviorTreeFactory,
    StatefulActionNode,
    SyncActionNode,
    NodeStatus,
    ports,
)


xml_text = """
 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
        <Sequence>
            <!-- Initialize the interpolated position -->
            <SetBlackboard output_key="interpolated" value="None" />

            <!-- Interpolate from the initial position to the final one printing
                 at each step. -->
            <ReactiveSequence name="root">
                <Print value="{interpolated}" />
                <Interpolate x0="[1.0, 0.0]" x1="[0.0, 1.0]" out="{interpolated}" />
            </ReactiveSequence>
        </Sequence>
     </BehaviorTree>

 </root>
"""


@ports(inputs=["x0", "x1"], outputs=["out"])
class Interpolate(StatefulActionNode):
    def on_start(self):
        self.t = 0.0
        self.x0 = np.asarray(self.get_input("x0"))
        self.x1 = np.asarray(self.get_input("x1"))
        return NodeStatus.RUNNING

    def on_running(self):
        if self.t < 1.0:
            x = (1.0 - self.t) * self.x0 + self.t * self.x1
            self.set_output("out", x)
            self.t += 0.1
            return NodeStatus.RUNNING
        else:
            return NodeStatus.SUCCESS

    def on_halted(self):
        pass


@ports(inputs=["value"])
class Print(SyncActionNode):
    def tick(self):
        print(self.get_input("value"))
        return NodeStatus.SUCCESS


factory = BehaviorTreeFactory()
factory.register(Interpolate)
factory.register(Print)

tree = factory.create_tree_from_text(xml_text)
tree.tick_while_running()
