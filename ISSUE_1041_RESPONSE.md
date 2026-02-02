# Response to Issue #1041: Performance Degradation with Parallel Nodes and Recursive Subtrees

Thank you for reporting this. We investigated the behavior you described and reviewed the internal state management of `ParallelNode` and related control nodes. Based on our analysis, we believe this is not a bug in the library itself, but rather a usage pattern issue related to tree lifecycle management. Let me explain in detail.

## 1. Always call `haltTree()` before re-ticking a completed or interrupted tree

When a tree finishes execution (returns `SUCCESS` or `FAILURE`), the `tickRoot()` method resets the root node's **status** back to `IDLE`, but it does **not** propagate a `halt()` signal through the entire tree. This means that if you immediately re-tick the tree without calling `tree.haltTree()` first, some internal nodes may still be carrying stale state from the previous execution cycle.

The correct pattern for repeated execution is:

```cpp
while (keep_running)
{
  auto status = tree.tickWhileRunning();
  // Process result...

  tree.haltTree();  // <-- Required before the next run
}
```

Or, if you are ticking manually:

```cpp
auto status = tree.tickOnce();
if (status != NodeStatus::RUNNING)
{
  tree.haltTree();
  // Now it is safe to start a new execution cycle
}
```

Omitting the `haltTree()` call between runs can cause internal bookkeeping structures (such as the completed child sets in parallel nodes) to accumulate residual state, which would manifest as the gradual performance degradation you observed.

## 2. Creating a new thread per tree execution without halting can leak RUNNING state

If your application spawns a new thread for each behavior tree execution cycle -- for example, launching a `std::thread` that calls `tickWhileRunning()` -- without first halting the previous execution, you can end up with nodes that remain in `RUNNING` status indefinitely. These orphaned RUNNING nodes are never cleaned up because no halt signal reaches them, and their associated state (completed child lists, counters, coroutine handles, etc.) persists for the lifetime of the `Tree` object.

Over many cycles, this leaked state accumulates and degrades performance. The fix is straightforward: always ensure the previous execution is fully halted before starting a new one, and avoid overlapping concurrent ticks on the same `Tree` instance. `Tree` is not thread-safe and must not be ticked from multiple threads simultaneously.

## 3. Parallel nodes DO clear their internal state on halt

We verified that `ParallelNode::halt()` calls `clear()`, which resets `completed_list_`, `success_count_`, and `failure_count_` to their initial values:

```cpp
void ParallelNode::halt()
{
  clear();           // clears completed_list_, success_count_, failure_count_
  ControlNode::halt();  // propagates halt to children
}

void ParallelNode::clear()
{
  completed_list_.clear();
  success_count_ = 0;
  failure_count_ = 0;
}
```

Additionally, when a `ParallelNode` reaches its success or failure threshold during `tick()`, it calls `clear()` and `resetChildren()` before returning. So under normal operation -- where `haltTree()` is called between execution cycles -- there is no path for `completed_list_` or the counters to grow unboundedly.

The degradation you are seeing is consistent with `halt()` never being called on the parallel nodes between runs, which would happen if `haltTree()` is not invoked as described in points 1 and 2 above.

## 4. Next steps: could you share a minimal reproduction?

If the issue persists after ensuring that `haltTree()` is called between every execution cycle and that no concurrent ticks overlap, we would like to investigate further. In that case, it would be very helpful if you could provide:

- A **minimal XML tree** that reproduces the degradation (ideally with parallel nodes and subtrees as you described).
- The **C++ code** showing how the tree is created, ticked, and (if applicable) run across threads.
- Approximate **timings**: how many tick cycles before degradation becomes noticeable, and what the symptom looks like (increased tick latency, memory growth, etc.).
- Your **BehaviorTree.CPP version** and compiler/platform information.

A self-contained example that we can build and run locally would let us confirm whether there is an underlying issue we missed or help pinpoint the exact pattern causing trouble in your setup.

We appreciate you taking the time to report this and are happy to help debug further.
