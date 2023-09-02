#!/usr/bin/env python3

"""
Top-level module of the BehaviorTree.CPP Python bindings.
"""

# re-export
from btpy_cpp import (
    BehaviorTreeFactory,
    NodeStatus,
    StatefulActionNode,
    SyncActionNode,
    Tree,
)


def ports(inputs=[], outputs=[]):
    """Decorator to specify input and outputs ports for an action node."""

    def specify_ports(cls):
        cls.input_ports = list(inputs)
        cls.output_ports = list(outputs)
        return cls

    return specify_ports


class AsyncActionNode(StatefulActionNode):
    """An abstract action node implemented via cooperative multitasking.

    Subclasses must implement the `run()` method as a generator.  Optionally,
    this method can return a final `NodeStatus` value to indicate its exit
    condition.

    Note:
        It is the responsibility of the action author to not block the main
        behavior tree loop with long-running tasks. `yield` calls should be
        placed whenever a pause is appropriate.
    """

    def __init__(self, name, config):
        super().__init__(name, config)

    def on_start(self):
        self.coroutine = self.run()
        return NodeStatus.RUNNING

    def on_running(self):
        # The library logic should never allow this to happen, but users can
        # still manually call `on_running` without an associated `on_start`
        # call. Make sure to print a useful error when this happens.
        if self.coroutine is None:
            raise "AsyncActionNode run without starting"

        # Resume the coroutine (generator). As long as the generator is not
        # exhausted, keep this action in the RUNNING state.
        try:
            next(self.coroutine)
            return NodeStatus.RUNNING
        except StopIteration as e:
            # If the action returns a status then propagate it upwards.
            if e.value is not None:
                return e.value
            # Otherwise, just assume the action finished successfully.
            else:
                return NodeStatus.SUCCESS
