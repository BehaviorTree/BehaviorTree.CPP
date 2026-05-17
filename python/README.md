# pybt

Python bindings for [BehaviorTree.CPP](https://github.com/BehaviorTree/BehaviorTree.CPP).

## Install

```bash
cd python
python -m venv .venv # Only needs to be done once.
source .venv/bin/activate
pip install -e .[dev]
python -c "import pybt; print(pybt.__version__, pybt._pybt.__phase__)"

```

Requires Python 3.9+ and a C++17 toolchain.

## Quick check

```python
import pybt
print(pybt.__version__)
```
