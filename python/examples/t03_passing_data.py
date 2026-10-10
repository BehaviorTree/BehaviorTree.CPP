"""t03 — Passing multiple values via separate ports.

Extends t02 with a stateful producer (counts ticks before reporting a
pose) and multiple primitive ports passed to a single consumer.

A later release adds `register_type` for sending custom Python classes
through a single port — until then, pass each field as its own primitive
port (string / int / float / bool).

Run: python t03_passing_data.py
Expected output:
    [navigate] heading to (1.5, 2.5) at 0.8 m/s
    final status: SUCCESS
"""

from __future__ import annotations

import pybt

XML = """
<root BTCPP_format="4">
  <BehaviorTree ID="Main">
    <Sequence>
      <PlanPose x="{x}" y="{y}" speed="{speed}"/>
      <Navigate x="{x}" y="{y}" speed="{speed}"/>
    </Sequence>
  </BehaviorTree>
</root>
"""


@pybt.ports(outputs=["x", "y", "speed"])
class PlanPose(pybt.SyncActionNode):
    def tick(self) -> pybt.NodeStatus:
        self.set_output("x", 1.5)
        self.set_output("y", 2.5)
        self.set_output("speed", 0.8)
        return pybt.NodeStatus.SUCCESS


@pybt.ports(inputs=["x", "y", "speed"])
class Navigate(pybt.SyncActionNode):
    def tick(self) -> pybt.NodeStatus:
        x = float(self.get_input("x"))
        y = float(self.get_input("y"))
        speed = float(self.get_input("speed"))
        print(f"[navigate] heading to ({x}, {y}) at {speed} m/s")
        return pybt.NodeStatus.SUCCESS


def main() -> int:
    factory = pybt.BehaviorTreeFactory()
    factory.register_node_type(PlanPose, "PlanPose")
    factory.register_node_type(Navigate, "Navigate")
    tree = factory.create_tree_from_text(XML)
    status = tree.tick_while_running()
    print(f"final status: {status.name}")
    return 0 if status == pybt.NodeStatus.SUCCESS else 1


if __name__ == "__main__":
    raise SystemExit(main())
