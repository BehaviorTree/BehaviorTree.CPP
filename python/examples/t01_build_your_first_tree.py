"""t01 — Build your first tree.

Shows the minimum needed to register a Python action, build a tree from
XML, and tick it to completion.

This tree runs two custom actions in sequence: a "check" that returns
SUCCESS or FAILURE based on a boolean, and a "say" that prints a line.

Run: python t01_build_your_first_tree.py
Expected output:
    [check] battery_ok=True
    [say] hello from pybt
    final status: SUCCESS
"""

from __future__ import annotations

import pybt

XML = """
<root BTCPP_format="4">
  <BehaviorTree ID="Main">
    <Sequence>
      <CheckBatteryOk/>
      <SaySomething/>
    </Sequence>
  </BehaviorTree>
</root>
"""


class CheckBatteryOk(pybt.SyncActionNode):
    def tick(self) -> pybt.NodeStatus:
        battery_ok = True
        print(f"[check] battery_ok={battery_ok}")
        return pybt.NodeStatus.SUCCESS if battery_ok else pybt.NodeStatus.FAILURE


class SaySomething(pybt.SyncActionNode):
    def tick(self) -> pybt.NodeStatus:
        print("[say] hello from pybt")
        return pybt.NodeStatus.SUCCESS


def main() -> int:
    factory = pybt.BehaviorTreeFactory()
    factory.register_node_type(CheckBatteryOk, "CheckBatteryOk")
    factory.register_node_type(SaySomething, "SaySomething")
    tree = factory.create_tree_from_text(XML)
    status = tree.tick_while_running()
    print(f"final status: {status.name}")
    return 0 if status == pybt.NodeStatus.SUCCESS else 1


if __name__ == "__main__":
    raise SystemExit(main())
