# pybt tests

## Setup (once)

```bash
cd python
python3 -m venv .venv
source .venv/bin/activate
pip install -e .[dev] -v
```

## Run

```bash
# Everything in tests/
pytest

# Only smoke tests
pytest -m smoke

# A single test
pytest tests/test_smoke.py::test_sync_action_node

# Standalone (no pytest, no fixtures, plain Python)
python tests/test_smoke.py
```

## What's here

| File | Covers |
|---|---|
| `test_smoke.py` | Module surface, exception hierarchy, sync/stateful nodes, ports, JSON bridge, fork safety, SIGINT, GIL release, module reload. |
| `test_lifecycle.py` | Subprocess-isolated interpreter-shutdown tests. Marked `slow`. |
| `conftest.py` | Shared fixtures (`fresh_factory`, `simple_xml`, `wrap_in_tree`) and an `assert_tree_returns` helper. |

## Related

- Microbenchmarks: see [`../benchmarks/README.md`](../benchmarks/README.md).
