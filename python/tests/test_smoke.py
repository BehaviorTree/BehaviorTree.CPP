"""Smoke tests for pybt.

Run any of:
    python python/tests/test_smoke.py
    pytest python/tests/test_smoke.py -v
    pytest python/tests/test_smoke.py::test_sync_action_node
"""

import multiprocessing
import sys

import pybt

XML_SINGLE = '<root BTCPP_format="4"><BehaviorTree>{}</BehaviorTree></root>'


# ---------------------------------------------------------------------------
# Module surface
# ---------------------------------------------------------------------------


def test_import_and_basic_types():
    """Module loads, enum values are distinct, version is non-empty."""
    assert pybt.NodeStatus.SUCCESS != pybt.NodeStatus.FAILURE
    assert pybt.NodeStatus.RUNNING != pybt.NodeStatus.IDLE
    assert pybt.NodeType.ACTION != pybt.NodeType.CONTROL
    assert pybt.PortDirection.INPUT != pybt.PortDirection.OUTPUT
    assert pybt.__version__


def test_exception_hierarchy():
    """BTError hierarchy is exposed and BTRuntimeError descends from BTError."""
    assert issubclass(pybt.BTRuntimeError, pybt.BTError)
    assert issubclass(pybt.BTLogicError, pybt.BTError)
    assert issubclass(pybt.BTNodeExecutionError, pybt.BTRuntimeError)


# ---------------------------------------------------------------------------
# Built-in (pure C++) tree
# ---------------------------------------------------------------------------


def test_built_in_always_success():
    """No Python nodes — pure C++ AlwaysSuccess ticks to SUCCESS."""
    f = pybt.BehaviorTreeFactory()
    t = f.create_tree_from_text(XML_SINGLE.format("<AlwaysSuccess/>"))
    assert t.tick_while_running() == pybt.NodeStatus.SUCCESS


# ---------------------------------------------------------------------------
# SyncActionNode subclass
# ---------------------------------------------------------------------------


def test_sync_action_node():
    """SyncActionNode subclass ticks via the adapter and returns user status."""
    visits = []

    class Foo(pybt.SyncActionNode):
        def tick(self):
            visits.append(self.name)
            return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Foo, "Foo")
    t = f.create_tree_from_text(XML_SINGLE.format("<Foo/>"))
    assert t.tick_while_running() == pybt.NodeStatus.SUCCESS
    assert visits == ["Foo"]


def test_sync_action_failure():
    """SyncActionNode returning FAILURE propagates."""

    class Nope(pybt.SyncActionNode):
        def tick(self):
            return pybt.NodeStatus.FAILURE

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Nope, "Nope")
    t = f.create_tree_from_text(XML_SINGLE.format("<Nope/>"))
    assert t.tick_while_running() == pybt.NodeStatus.FAILURE


# ---------------------------------------------------------------------------
# StatefulActionNode lifecycle
# ---------------------------------------------------------------------------


def test_stateful_action_lifecycle():
    """on_start once, on_running until SUCCESS, events in correct order."""
    events = []

    class Counter(pybt.StatefulActionNode):
        def __init__(self, name, config):
            super().__init__(name, config)
            self.n = 0

        def on_start(self):
            events.append("start")
            return pybt.NodeStatus.RUNNING

        def on_running(self):
            self.n += 1
            events.append(f"run:{self.n}")
            return pybt.NodeStatus.SUCCESS if self.n >= 3 else pybt.NodeStatus.RUNNING

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Counter, "Counter")
    t = f.create_tree_from_text(XML_SINGLE.format("<Counter/>"))
    assert t.tick_while_running(sleep_ms=1) == pybt.NodeStatus.SUCCESS
    assert events == ["start", "run:1", "run:2", "run:3"]


def test_stateful_on_halted_optional():
    """StatefulActionNode without on_halted defined still works (no-op fallback)."""

    class NoCleanup(pybt.StatefulActionNode):
        def on_start(self):
            return pybt.NodeStatus.SUCCESS

        def on_running(self):
            return pybt.NodeStatus.SUCCESS
        # Intentionally no on_halted.

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(NoCleanup, "NoCleanup")
    t = f.create_tree_from_text(XML_SINGLE.format("<NoCleanup/>"))
    assert t.tick_while_running() == pybt.NodeStatus.SUCCESS


# ---------------------------------------------------------------------------
# Simple-action callback
# ---------------------------------------------------------------------------


def test_simple_action_callback():
    """register_simple_action wraps a plain Python callable."""
    invoked = [0]

    def cb(node):
        invoked[0] += 1
        return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    f.register_simple_action("Cb", cb)
    t = f.create_tree_from_text(XML_SINGLE.format("<Cb/>"))
    assert t.tick_while_running() == pybt.NodeStatus.SUCCESS
    assert invoked[0] == 1


# ---------------------------------------------------------------------------
# Exception translation
# ---------------------------------------------------------------------------


def test_python_exception_propagates():
    """RuntimeError raised in Python tick() surfaces as an exception out of tick_while_running."""

    class Boom(pybt.SyncActionNode):
        def tick(self):
            raise RuntimeError("kaboom")

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Boom, "Boom")
    t = f.create_tree_from_text(XML_SINGLE.format("<Boom/>"))
    try:
        t.tick_while_running()
    except Exception as e:
        assert "kaboom" in str(e), f"unexpected message: {e!r}"
        return
    raise AssertionError("tick_while_running should have raised")


# ---------------------------------------------------------------------------
# Ports (JSON bridge)
# ---------------------------------------------------------------------------


def test_input_port_xml_string_literal():
    """A string literal declared in XML reaches Python via get_input."""
    received = []

    @pybt.ports(inputs=["msg"])
    class Echo(pybt.SyncActionNode):
        def tick(self):
            received.append(self.get_input("msg"))
            return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Echo, "Echo")
    t = f.create_tree_from_text(
        XML_SINGLE.format('<Echo msg="hello"/>'),
    )
    t.tick_while_running()
    assert received == ["hello"]


def test_output_then_input_roundtrip():
    """Writer sets a blackboard entry; reader reads it back via the JSON bridge."""
    seen = []

    @pybt.ports(outputs=["value"])
    class Writer(pybt.SyncActionNode):
        def tick(self):
            self.set_output("value", 42)
            return pybt.NodeStatus.SUCCESS

    @pybt.ports(inputs=["value"])
    class Reader(pybt.SyncActionNode):
        def tick(self):
            seen.append(self.get_input("value"))
            return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Writer, "Writer")
    f.register_node_type(Reader, "Reader")
    xml = XML_SINGLE.format(
        '<Sequence>'
        '<Writer value="{shared}"/>'
        '<Reader value="{shared}"/>'
        '</Sequence>'
    )
    t = f.create_tree_from_text(xml)
    assert t.tick_while_running() == pybt.NodeStatus.SUCCESS
    assert seen == [42]


def test_set_output_undeclared_port_raises():
    """set_output on a port that wasn't declared raises BTRuntimeError."""

    class BadWriter(pybt.SyncActionNode):
        def tick(self):
            self.set_output("never_declared", 1)
            return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(BadWriter, "BadWriter")
    t = f.create_tree_from_text(XML_SINGLE.format("<BadWriter/>"))
    try:
        t.tick_while_running()
    except Exception as e:
        assert "never_declared" in str(e), f"unexpected message: {e!r}"
        return
    raise AssertionError("set_output of an undeclared port should have raised")


# ---------------------------------------------------------------------------
# Fork safety
# ---------------------------------------------------------------------------


def _fork_child_target(queue):
    """Run inside a forked child — should fail with BTRuntimeError."""
    import pybt as _pybt

    try:
        f = _pybt.BehaviorTreeFactory()
        t = f.create_tree_from_text(
            '<root BTCPP_format="4"><BehaviorTree><AlwaysSuccess/></BehaviorTree></root>'
        )
        t.tick_while_running()
        queue.put(("unexpected_success", None))
    except _pybt.BTRuntimeError as e:
        queue.put(("BTRuntimeError", str(e)))
    except BaseException as e:
        queue.put((type(e).__name__, str(e)))


def test_fork_safety_raises_btruntimeerror():
    """A tree.tick_while_running() in a forked child raises BTRuntimeError, not a crash."""
    if sys.platform == "win32":
        return  # No fork on Windows
    # Parent must have imported pybt and at least registered the startup pid.
    # The startup pid is captured by module init; we trigger it by touching the module.
    _ = pybt.BehaviorTreeFactory()

    ctx = multiprocessing.get_context("fork")
    q = ctx.Queue()
    p = ctx.Process(target=_fork_child_target, args=(q,))
    p.start()
    p.join(timeout=5)
    assert p.exitcode is not None, "child process hung"
    kind, msg = q.get(timeout=1)
    assert kind == "BTRuntimeError", f"expected BTRuntimeError, got {kind}: {msg}"
    assert "fork-safe" in msg, f"unexpected message: {msg}"


# ---------------------------------------------------------------------------
# Standalone runner
# ---------------------------------------------------------------------------


def _collect_tests():
    g = globals()
    return [(n, g[n]) for n in sorted(g) if n.startswith("test_") and callable(g[n])]


def _run_all():
    tests = _collect_tests()
    passed = 0
    failed = []
    for name, fn in tests:
        try:
            fn()
        except BaseException as e:
            failed.append((name, e))
            print(f"FAIL  {name}: {type(e).__name__}: {e}")
        else:
            passed += 1
            print(f"PASS  {name}")
    print()
    print(f"{passed}/{len(tests)} passed", "" if not failed else f"({len(failed)} failed)")
    return 0 if not failed else 1


if __name__ == "__main__":
    sys.exit(_run_all())
