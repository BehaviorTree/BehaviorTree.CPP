"""This script will scan `python/src/pybt/**.py`,
`python/examples/**.py`, and the built Sphinx HTML for forbidden C++
references (`BT::`, `.cpp`, `.hpp`, `include/behaviortree_cpp`, etc.) per
the Documentation Standards in the plan. README files are allowlisted.
"""

import sys


def main() -> int:
    return 0


if __name__ == "__main__":
    sys.exit(main())
