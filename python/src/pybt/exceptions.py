"""pybt exception hierarchy, re-exported from the C++ binding.

All BT.CPP exceptions translate into one of these types when they cross
the binding boundary. Catch `BTError` to handle any pybt-originated failure.
"""

from ._pybt import (
    BTError,
    BTLogicError,
    BTNodeExecutionError,
    BTRuntimeError,
)

__all__ = [
    "BTError",
    "BTLogicError",
    "BTNodeExecutionError",
    "BTRuntimeError",
]
