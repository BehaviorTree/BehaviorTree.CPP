"""Microbenchmarks for pybt.

Run:
    pytest benchmarks/ --benchmark-only

Each benchmark records wall-clock per call. Captures baselines
only, regression thresholds are TODO for later.

Three signals:
  1. tick_rate_100_node_tree    — pure-C++ throughput (no Python overhead).
  2. port_get_set_latency       — round-trip get_input + set_output cost.
  3. python_node_overhead_*     — delta between (1) and a 1-Python-node tree.
"""

import pytest

import pybt

pytestmark = pytest.mark.benchmark


XML_ROOT = '<root BTCPP_format="4"><BehaviorTree>{}</BehaviorTree></root>'


# ---------------------------------------------------------------------------
# 1. Tick rate of a 100-node pure-C++ tree
# ---------------------------------------------------------------------------
def bench_tick_rate_100_node_tree(benchmark):
    """Wall-clock per tick of a 100-leaf Sequence of AlwaysSuccess (no Python)."""
    children = "<AlwaysSuccess/>" * 100
    xml = XML_ROOT.format(f"<Sequence>{children}</Sequence>")
    factory = pybt.BehaviorTreeFactory()
    tree = factory.create_tree_from_text(xml)

    def one_tick():
        # Re-tick: trees terminating in SUCCESS need a fresh state, but
        # tick_while_running internally handles this since the root is
        # idempotent across SUCCESS returns.
        tree.tick_while_running()

    benchmark(one_tick)


# ---------------------------------------------------------------------------
# 2. Port get/set latency
# ---------------------------------------------------------------------------
def bench_port_get_set_latency(benchmark):
    """Cost of one `get_input` + one `set_output` round trip inside a Python tick."""

    @pybt.ports(inputs=["in"], outputs=["out"])
    class Echo(pybt.SyncActionNode):
        def tick(self):
            self.set_output("out", self.get_input("in"))
            return pybt.NodeStatus.SUCCESS

    factory = pybt.BehaviorTreeFactory()
    factory.register_node_type(Echo, "Echo")
    xml = XML_ROOT.format('<Echo in="42" out="{r}"/>')
    tree = factory.create_tree_from_text(xml)

    benchmark(tree.tick_while_running)


# ---------------------------------------------------------------------------
# 3. Python-node overhead vs C++ baseline (paired benchmarks)
# ---------------------------------------------------------------------------
def bench_python_node_overhead_cpp_baseline(benchmark):
    """Wall-clock baseline: single C++ AlwaysSuccess tick."""
    factory = pybt.BehaviorTreeFactory()
    tree = factory.create_tree_from_text(XML_ROOT.format("<AlwaysSuccess/>"))
    benchmark(tree.tick_while_running)


def bench_python_node_overhead_py(benchmark):
    """Wall-clock with one Python SyncActionNode — delta from baseline = overhead."""

    class Noop(pybt.SyncActionNode):
        def tick(self):
            return pybt.NodeStatus.SUCCESS

    factory = pybt.BehaviorTreeFactory()
    factory.register_node_type(Noop, "Noop")
    tree = factory.create_tree_from_text(XML_ROOT.format("<Noop/>"))
    benchmark(tree.tick_while_running)
