#!/usr/bin/env python3

"""
Demonstration of passing generic data between nodes.
"""

import numpy as np
from btpy import BehaviorTreeFactory, SyncActionNode, NodeStatus, ports


xml_text = """
 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
         <Sequence name="root">
             <AlwaysSuccess/>
             <Rotate    position="[1.0, 0.0]" theta="90."         out="{rotated}" />
             <Translate position="{rotated}"  offset="[0.1, 0.1]" out="{translated}" />
             <Print     value="{translated}" />
         </Sequence>
     </BehaviorTree>

 </root>
"""


@ports(inputs=["position", "theta"], outputs=["out"])
class Rotate(SyncActionNode):
    def tick(self):
        # Build a rotation matrix which rotates points by `theta` degrees.
        theta = np.deg2rad(self.get_input("theta"))
        c, s = np.cos(theta), np.sin(theta)
        M = np.array([[c, -s], [s, c]])

        # Apply the rotation to the input position.
        position = self.get_input("position")
        rotated = M @ position

        # Set the output.
        self.set_output("out", rotated)

        return NodeStatus.SUCCESS


@ports(inputs=["position", "offset"], outputs=["out"])
class Translate(SyncActionNode):
    def tick(self):
        offset = np.asarray(self.get_input("offset"))

        # Apply the translation to the input position.
        position = np.asarray(self.get_input("position"))
        translated = position + offset

        # Set the output.
        self.set_output("out", translated)

        return NodeStatus.SUCCESS


@ports(inputs=["value"])
class Print(SyncActionNode):
    def tick(self):
        value = self.get_input("value")
        if value is not None:
            print(value)
        return NodeStatus.SUCCESS


factory = BehaviorTreeFactory()
factory.register(Rotate)
factory.register(Translate)
factory.register(Print)

tree = factory.create_tree_from_text(xml_text)
tree.tick_while_running()
