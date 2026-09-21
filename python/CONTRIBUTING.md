# Contributing to pybt

pybt is the Python binding for [BehaviorTree.CPP](https://github.com/BehaviorTree/BehaviorTree.CPP). This page covers local development; user-facing install lives in [README.md](README.md).

## Setup (once per clone)

```bash
cd python
python3 -m venv .venv
source .venv/bin/activate
pip install -e .[dev] -v
```

Requires Python 3.12+ and a C++17 toolchain (the build compiles the bundled BehaviorTree.CPP source).

## Run tests

```bash
pytest                                 # everything in tests/
pytest -m smoke                        # only the smoke gate (fast)
pytest tests/test_smoke.py::test_sync_action_node -v
python tests/test_smoke.py             # standalone runner, no pytest
```

## Run benchmarks

```bash
pytest benchmarks/ --benchmark-only
```

Results written in `benchmarks/.benchmarks/` (git-ignored). See [`benchmarks/README.md`](benchmarks/README.md).

## Run examples

```bash
python examples/t01_build_your_first_tree.py
python examples/t02_basic_ports.py
python examples/t03_passing_data.py
```

Each script prints what it's doing and exits 0 on success. `pytest tests/test_examples.py` runs all three in subprocesses and asserts clean exits — that's the rot-prevention gate.

## Pre-commit hooks

Install once:

```bash
pip install pre-commit
pre-commit install   # in the repo root
```

This runs `ruff`, `mypy`, the no-C++-refs check, and the project's standard hooks before each commit.

## CI

`.github/workflows/python_test.yml` mirrors the local pytest invocation across Python 3.12, 3.13, and 3.13t (free-threaded, allowed to fail). Two extra jobs run a standalone CMake build under default flags and under `-fvisibility=hidden -flto` (the manylinux configuration) to catch symbol-hygiene regressions.

## Where things live

| Path | What |
|---|---|
| `src/pybt/` | Python package (the user-facing surface) |
| `src/_pybt/` | C++ binding code (nanobind) |
| `tests/` | pytest suite — smoke + lifecycle + example runner |
| `examples/` | Runnable tutorial scripts (t01..t03 so far) |
| `benchmarks/` | pytest-benchmark microbenchmarks |
| `docs/` | Sphinx site (stub for now) |
| `pyproject.toml` | Build config (scikit-build-core, nanobind, pytest) |
| `CMakeLists.txt` | nanobind extension + CTest regression guards |

## Style

- Python: `ruff` for lint and format, `mypy` for types.
- C++: project root `.clang-format` (Google C++ with 2-space indent, 90-char line limit).
