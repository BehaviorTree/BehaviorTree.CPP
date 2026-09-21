"""Pythonic helpers for declaring custom nodes and ports.

These wrap the underlying nanobind bindings (`pybt.BehaviorTreeFactory.register_node_type`
and `register_simple_action`) with class- and function-decorator forms.
"""

from collections.abc import Callable, Iterable
from typing import Optional

from ._pybt import BehaviorTreeFactory


def ports(
    *,
    inputs: Optional[Iterable[str]] = None,
    outputs: Optional[Iterable[str]] = None,
):
    """Declare a custom node's input and output ports.

    Attach the returned decorator to a `SyncActionNode` or `StatefulActionNode`
    subclass. The factory's `register_node_type` reads `cls.input_ports`
    and `cls.output_ports` set here.

    Example::

        @pybt.ports(inputs=["target"], outputs=["result"])
        class Approach(pybt.SyncActionNode):
            def tick(self):
                self.set_output("result", "done")
                return pybt.NodeStatus.SUCCESS
    """
    inputs_list = list(inputs) if inputs else []
    outputs_list = list(outputs) if outputs else []

    def decorator(cls):
        cls.input_ports = inputs_list
        cls.output_ports = outputs_list
        return cls

    return decorator


def simple_action(
    factory: BehaviorTreeFactory,
    id: str,
    ports: Optional[Iterable[tuple]] = None,
):
    """Register a callable on `factory` as a synchronous action node.

    The wrapped function receives a `TreeNode` and must return a `NodeStatus`.

    Example::

        factory = pybt.BehaviorTreeFactory()

        @pybt.simple_action(factory, "Hello")
        def hello(node):
            return pybt.NodeStatus.SUCCESS
    """

    def decorator(fn: Callable):
        factory.register_simple_action(id, fn, list(ports) if ports else None)
        return fn

    return decorator
