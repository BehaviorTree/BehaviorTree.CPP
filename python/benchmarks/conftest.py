"""Benchmark suite configuration — defaults storage to ./benchmarks/.benchmarks."""

import os


def pytest_configure(config):
    """Default `--benchmark-storage` to the benchmarks/.benchmarks directory.

    User can still override on the command line. Without this, pytest-benchmark
    writes to the pytest rootdir (`python/.benchmarks`), which would be
    confusing in a multi-suite layout.
    """
    if not hasattr(config.option, "benchmark_storage"):
        return  # pytest-benchmark not installed; nothing to do.
    if config.option.benchmark_storage:
        return  # user supplied a path; don't override.
    storage_dir = os.path.join(os.path.dirname(__file__), ".benchmarks")
    config.option.benchmark_storage = f"file://{storage_dir}"
