"""Smoke tests for pybt.

Run any of:
    python python/tests/test_smoke.py
    pytest python/tests/test_smoke.py -v
    pytest -m smoke
    pytest python/tests/test_smoke.py::test_sync_action_node
"""

import multiprocessing
import sys

import pytest

import pybt

# Every test in this file participates in the `-m smoke` selection.
pytestmark = pytest.mark.smoke

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


def test_simple_action_decorator():
    """@pybt.simple_action(factory, id) decorator registers the wrapped function."""
    f = pybt.BehaviorTreeFactory()
    invoked = [0]

    @pybt.simple_action(f, "Deco")
    def deco(node):
        invoked[0] += 1
        return pybt.NodeStatus.SUCCESS

    t = f.create_tree_from_text(XML_SINGLE.format("<Deco/>"))
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
# Registration validation
# ---------------------------------------------------------------------------


def test_register_non_action_class_raises():
    """Registering a class that does not derive from SyncActionNode/StatefulActionNode fails clearly.

    The error may surface at registration or at first tree construction (when
    the factory's builder tries to cast the Python instance to a C++ TreeNode).
    Either is acceptable; what matters is that a Python exception is raised
    rather than a segfault.
    """

    class NotANode:
        def tick(self):
            return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    try:
        f.register_node_type(NotANode, "NotANode")
        t = f.create_tree_from_text(XML_SINGLE.format("<NotANode/>"))
        t.tick_while_running()
    except Exception:
        return
    raise AssertionError(
        "registering or ticking a non-action class should have raised"
    )


def test_register_duplicate_id_raises():
    """Registering two node types with the same ID raises (does not silently replace)."""

    class A(pybt.SyncActionNode):
        def tick(self):
            return pybt.NodeStatus.SUCCESS

    class B(pybt.SyncActionNode):
        def tick(self):
            return pybt.NodeStatus.SUCCESS

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(A, "Dup")
    try:
        f.register_node_type(B, "Dup")
    except Exception:
        return
    raise AssertionError("duplicate-id registration should have raised")


def test_create_tree_from_malformed_xml_raises():
    """Malformed XML raises a clear Python exception, not a crash."""
    f = pybt.BehaviorTreeFactory()
    try:
        f.create_tree_from_text("<this is not valid xml")
    except Exception:
        return
    raise AssertionError("malformed XML should have raised")


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
# SIGINT / Ctrl-C
# ---------------------------------------------------------------------------

def test_sigint_interrupts_tick():
    """SIGINT during tick_while_running raises KeyboardInterrupt within ~50ms."""
    import signal
    import threading
    import time

    class Forever(pybt.StatefulActionNode):
        def on_start(self):
            return pybt.NodeStatus.RUNNING

        def on_running(self):
            return pybt.NodeStatus.RUNNING

    f = pybt.BehaviorTreeFactory()
    f.register_node_type(Forever, "Forever")
    t = f.create_tree_from_text(XML_SINGLE.format("<Forever/>"))

    # Schedule SIGINT to self after 50ms. signal.raise_signal targets the
    # current process; CPython delivers the resulting signal to the main
    # thread, where tick_while_running's PyErr_CheckSignals picks it up.
    timer = threading.Timer(0.05, lambda: signal.raise_signal(signal.SIGINT))
    timer.start()

    start = time.monotonic()
    try:
        t.tick_while_running(sleep_ms=5)
    except KeyboardInterrupt:
        elapsed = time.monotonic() - start
        # Be generous: the per-iter signal check + sleep_ms can stack.
        assert elapsed < 1.0, f"SIGINT took too long: {elapsed:.3f}s"
        return
    finally:
        timer.cancel()
    raise AssertionError("tick_while_running should have raised KeyboardInterrupt")


# ---------------------------------------------------------------------------
# GIL release — two threads, two trees, concurrent
# ---------------------------------------------------------------------------

def test_two_threads_two_trees_run_concurrently():
    """Two threads ticking two trees in parallel finish in ~one tree's worth of wall time."""
    import threading
    import time

    class Burner(pybt.StatefulActionNode):
        def __init__(self, name, config):
            super().__init__(name, config)
            self.iters = 0

        def on_start(self):
            return pybt.NodeStatus.RUNNING

        def on_running(self):
            self.iters += 1
            return (
                pybt.NodeStatus.SUCCESS
                if self.iters >= 8
                else pybt.NodeStatus.RUNNING
            )

    def run_tree():
        f = pybt.BehaviorTreeFactory()
        f.register_node_type(Burner, "Burner")
        t = f.create_tree_from_text(XML_SINGLE.format("<Burner/>"))
        t.tick_while_running(sleep_ms=10)

    # Single-thread baseline first (fewer iters to keep test snappy).
    one_start = time.monotonic()
    run_tree()
    one_elapsed = time.monotonic() - one_start

    # Two threads in parallel.
    two_start = time.monotonic()
    t1 = threading.Thread(target=run_tree)
    t2 = threading.Thread(target=run_tree)
    t1.start()
    t2.start()
    t1.join()
    t2.join()
    two_elapsed = time.monotonic() - two_start

    # If the GIL were held throughout, two_elapsed ≈ 2 * one_elapsed.
    # With proper GIL release, two_elapsed ≈ one_elapsed.
    # Allow generous headroom (CI is noisy): pass if two-thread is < 1.7× single.
    ratio = two_elapsed / max(one_elapsed, 1e-6)
    assert ratio < 1.7, (
        f"two threads took {two_elapsed:.3f}s vs single {one_elapsed:.3f}s "
        f"(ratio {ratio:.2f}) — GIL probably not released during ticks"
    )


# ---------------------------------------------------------------------------
# Module reload
# ---------------------------------------------------------------------------

def test_module_reload_does_not_crash():
    """importlib.reload(pybt) either succeeds or raises cleanly — must not segfault."""
    import importlib

    try:
        importlib.reload(pybt)
    except (ImportError, RuntimeError) as e:
        # Some nanobind versions reject reload of extension modules — that's
        # acceptable as long as it raises cleanly rather than crashing.
        print(f"  (reload raised cleanly: {type(e).__name__}: {e})")
    # Module must still be functional afterward.
    assert pybt.NodeStatus.SUCCESS != pybt.NodeStatus.FAILURE


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
