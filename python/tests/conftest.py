"""Shared fixtures and helpers for the pybt test suite."""

import pytest

import pybt


@pytest.fixture
def fresh_factory():
    """A brand-new, empty BehaviorTreeFactory. Use per-test to avoid registration leakage."""
    return pybt.BehaviorTreeFactory()


@pytest.fixture
def simple_xml():
    """Trivial XML: one built-in AlwaysSuccess node inside a BehaviorTree."""
    return (
        '<root BTCPP_format="4">'
        '<BehaviorTree><AlwaysSuccess/></BehaviorTree>'
        "</root>"
    )


@pytest.fixture
def wrap_in_tree():
    """Return a helper that wraps an inner XML snippet in a single BehaviorTree."""

    def _wrap(inner: str) -> str:
        return (
            '<root BTCPP_format="4">'
            f"<BehaviorTree>{inner}</BehaviorTree>"
            "</root>"
        )

    return _wrap


def assert_tree_returns(tree, expected_status, sleep_ms: int = 10):
    """Tick the tree until it stops and assert it returned the expected status."""
    actual = tree.tick_while_running(sleep_ms=sleep_ms)
    assert actual == expected_status, (
        f"expected {expected_status}, got {actual}"
    )
