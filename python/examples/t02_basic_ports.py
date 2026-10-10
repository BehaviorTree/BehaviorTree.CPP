"""t02 — Basic ports.

Shows how to declare input and output ports on a custom action and pass
data between nodes through the blackboard.

The producer writes a string to its `out` port; the consumer reads the
same value from its `in` port and prints it.

Run: python t02_basic_ports.py
Expected output:
    [consume] got: hello world
    final status: SUCCESS
"""

from __future__ import annotations

import pybt

XML = """
<root BTCPP_format="4">
  <BehaviorTree ID="Main">
    <Sequence>
      <Produce out="{shared}"/>
      <Consume in="{shared}"/>
    </Sequence>
  </BehaviorTree>
</root>
"""


@pybt.ports(outputs=["out"])
class Produce(pybt.SyncActionNode):
    def tick(self) -> pybt.NodeStatus:
        self.set_output("out", "hello world")
        return pybt.NodeStatus.SUCCESS


@pybt.ports(inputs=["in"])
class Consume(pybt.SyncActionNode):
    def tick(self) -> pybt.NodeStatus:
        value = self.get_input("in")
        print(f"[consume] got: {value}")
        return pybt.NodeStatus.SUCCESS


def main() -> int:
    factory = pybt.BehaviorTreeFactory()
    factory.register_node_type(Produce, "Produce")
    factory.register_node_type(Consume, "Consume")
    tree = factory.create_tree_from_text(XML)
    status = tree.tick_while_running()
    print(f"final status: {status.name}")
    return 0 if status == pybt.NodeStatus.SUCCESS else 1


if __name__ == "__main__":
    raise SystemExit(main())
