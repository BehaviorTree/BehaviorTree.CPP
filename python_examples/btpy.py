#!/usr/bin/env python3

"""
Top-level module of the BehaviorTree.CPP Python bindings.
"""

from btpy_cpp import *  # re-export


def ports(inputs=[], outputs=[]):
    """Decorator to specify input and outputs ports for an action node."""

    def specify_ports(cls):
        cls.input_ports = list(inputs)
        cls.output_ports = list(outputs)
        return cls

    return specify_ports
