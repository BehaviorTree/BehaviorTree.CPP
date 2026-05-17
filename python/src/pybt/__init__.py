"""pybt — Python bindings for BehaviorTree.CPP."""

from importlib.metadata import PackageNotFoundError
from importlib.metadata import version as _pkg_version

from ._pybt import *  # noqa: F401, F403

try:
    __version__ = _pkg_version("pybt")
except PackageNotFoundError:
    __version__ = "0.0.0+unknown"
