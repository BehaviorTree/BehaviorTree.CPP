# Preempt

`Preempt` marks the point where a new branch of a reactive tree starts. Without it,
a reactive node can only halt the siblings of a child *after* that child returned
RUNNING, so an asynchronous action started in `onStart()` runs for a moment next to
the action that is about to be preempted (issue #1031). With the ROS action wrappers
that means a new goal is sent before the previous one is cancelled.

Wrap the action that starts the new branch, and the RUNNING branches of the reactive
ancestors above it are halted first:

```xml
<ReactiveSequence>
  <Fallback>
    <BatteryOk/>
    <Preempt>
      <GotoCharging/>
    </Preempt>
  </Fallback>
  <GotoKitchen/>
</ReactiveSequence>
```

When `battery_ok` turns false, `GotoKitchen` is halted before `GotoCharging` starts.

The preemption runs on the first tick of `Preempt` in each activation, that is,
whenever the node is not already RUNNING or completed. It walks up through the
parents and stops at `ReactiveSequence` and `ReactiveFallback`; the children of a
`Parallel` are left alone, since that node is allowed to keep several of them
RUNNING. `Preempt` is opt-in: a tree that does not use it behaves exactly as before.

Note that `Preempt` is a builtin node, so registering a custom node with that ID will
now throw.
