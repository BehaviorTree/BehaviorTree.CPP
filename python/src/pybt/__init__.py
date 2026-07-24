"""pybt — Python bindings for BehaviorTree.CPP."""

from importlib.metadata import PackageNotFoundError
from importlib.metadata import version as _pkg_version

from ._pybt import (
    BehaviorTreeFactory,
    BTError,
    BTLogicError,
    BTNodeExecutionError,
    BTRuntimeError,
    NodeStatus,
    NodeType,
    PortDirection,
    PortInfo,
    StatefulActionNode,
    SyncActionNode,
    Tree,
    TreeNode,
    bidirectional_port,
    input_port,
    output_port,
)
from .nodes import ports, simple_action

try:
    __version__ = _pkg_version("pybt")
except PackageNotFoundError:
    __version__ = "0.0.0+unknown"

__all__ = [
    "BehaviorTreeFactory",
    "BTError",
    "BTLogicError",
    "BTNodeExecutionError",
    "BTRuntimeError",
    "NodeStatus",
    "NodeType",
    "PortDirection",
    "PortInfo",
    "StatefulActionNode",
    "SyncActionNode",
    "Tree",
    "TreeNode",
    "__version__",
    "bidirectional_port",
    "input_port",
    "output_port",
    "ports",
    "simple_action",
]
