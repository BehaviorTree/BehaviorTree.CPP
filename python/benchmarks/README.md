# pybt benchmarks

Microbenchmarks for tracking pybt runtime performance.

## Run

```bash
cd python
pytest benchmarks/ --benchmark-only
```

Results written in `benchmarks/.benchmarks/` (git-ignored). Compare runs with:

```bash
pytest-benchmark compare
```

## What we measure

| Benchmark | What it captures |
|---|---|
| `bench_tick_rate_100_node_tree` | Pure-C++ throughput: how fast a 100-leaf tree can tick when no Python is involved. |
| `bench_port_get_set_latency` | Round-trip cost of one `get_input` + one `set_output` inside a Python tick (the JSON-bridge tax). |
| `bench_python_node_overhead_cpp_baseline` | Baseline: one C++ `AlwaysSuccess` tick. |
| `bench_python_node_overhead_py` | One Python `SyncActionNode` tick. The delta vs the baseline is the per-Python-node overhead. |

Total runtime: under 30 seconds on a typical laptop.

## baseline.json

`baseline.json` is the committed reference.
