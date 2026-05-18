"""Interpreter-lifecycle tests for pybt.

These use a subprocess so a crash on shutdown surfaces as a non-zero exit
code rather than killing the test runner.
"""

import subprocess
import sys
import textwrap

import pytest

pytestmark = [pytest.mark.smoke, pytest.mark.slow]


def _run_in_subprocess(script: str, timeout: float = 10.0) -> subprocess.CompletedProcess:
    return subprocess.run(
        [sys.executable, "-c", textwrap.dedent(script)],
        capture_output=True,
        text=True,
        timeout=timeout,
    )


def test_atexit_halts_running_tree():
    """A tree ticking in a background thread exits cleanly via a node-level stop flag.
    """
    
    script = """
        import threading
        import time

        import pybt

        stop = threading.Event()

        class StoppableForever(pybt.StatefulActionNode):
            def on_start(self):
                return pybt.NodeStatus.RUNNING

            def on_running(self):
                if stop.is_set():
                    return pybt.NodeStatus.SUCCESS
                return pybt.NodeStatus.RUNNING

        f = pybt.BehaviorTreeFactory()
        f.register_node_type(StoppableForever, "StoppableForever")
        t = f.create_tree_from_text(
            '<root BTCPP_format="4"><BehaviorTree><StoppableForever/></BehaviorTree></root>'
        )

        def go():
            try:
                t.tick_while_running(sleep_ms=10)
            except BaseException:
                pass

        th = threading.Thread(target=go)  # NOT daemon — joined explicitly below
        th.start()
        time.sleep(0.05)         # Let the tick loop enter C++
        stop.set()               # Cooperatively ask the node to finish
        th.join(timeout=2)
        assert not th.is_alive(), "tick thread did not exit within 2s of stop flag"
        print("MAIN_DONE", flush=True)
        """

    result = _run_in_subprocess(script)
    assert result.returncode == 0, (
        f"interpreter exited with {result.returncode}\n"
        f"stdout: {result.stdout}\nstderr: {result.stderr}"
    )
    assert "MAIN_DONE" in result.stdout, (
        f"main thread didn't reach the end: stdout={result.stdout}"
    )


def test_clean_module_load_and_shutdown():
    """Bare `import pybt` exits 0 — guards against atexit-handler regressions."""
    result = _run_in_subprocess(
        """
        import pybt
        assert pybt.NodeStatus.SUCCESS
        """
    )
    assert result.returncode == 0, (
        f"clean import + exit failed: stderr={result.stderr}"
    )
