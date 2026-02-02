# ParallelAll Node

## Overview

The `ParallelAll` node is a control flow node that executes all its children **concurrently** (but not in separate threads). It differs from the `Parallel` node in that it will always complete the execution of **all** its children, whereas `Parallel` may stop and halt other children once a certain number of successes or failures is reached.

## Behavior

The `ParallelAll` node ticks all of its children in sequence during each tick. It continues ticking until all children have completed (returned either SUCCESS, FAILURE, or SKIPPED).

### Return Status

- Returns `RUNNING` if at least one child is still running
- Returns `FAILURE` if the number of children that returned FAILURE is **greater than or equal to** the `failure_threshold`
- Returns `SUCCESS` if all children have completed and the failure count is below the threshold
- Returns `SKIPPED` if all children returned SKIPPED

## Parameters

### failure_threshold

- **Type**: `int`
- **Default**: `1`
- **Description**: If the number of children returning FAILURE is greater than or equal to this value, ParallelAll returns FAILURE.

The threshold uses a **greater than or equal to (>=)** comparison, meaning:
- `failure_threshold=1`: The node returns FAILURE if 1 or more children fail
- `failure_threshold=2`: The node returns FAILURE if 2 or more children fail
- `failure_threshold=3`: The node returns FAILURE if 3 or more children fail

### Negative Index Support

Like Python indexing, negative values for `failure_threshold` are supported:
- `-1` is equivalent to the number of children (all children must fail)
- `-2` is equivalent to `(number of children - 1)`

## XML Example

```xml
<root BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <ParallelAll failure_threshold="2">
      <Action1/>
      <Action2/>
      <Action3/>
    </ParallelAll>
  </BehaviorTree>
</root>
```

In this example:
- All three actions will be executed concurrently
- If 2 or more actions fail, the node returns FAILURE
- If 0 or 1 action fails, the node returns SUCCESS (assuming all complete)

## Use Cases

The `ParallelAll` node is useful when you need to:
1. Execute multiple actions simultaneously and wait for all to complete
2. Allow for a certain tolerance of failures before considering the overall operation as failed
3. Perform parallel operations where completion of all tasks is important

## Comparison with Parallel Node

| Feature | ParallelAll | Parallel |
|---------|-------------|----------|
| Executes all children | Yes, always | No, may halt early |
| Can stop early on success | No | Yes, with `success_count` |
| Can stop early on failure | No | Yes, with `failure_count` |
| Use case | When all tasks must complete | When early termination is desired |

## Implementation Notes

- Children are not executed in parallel threads; they are ticked sequentially
- The node maintains state between ticks for asynchronous actions
- Children that complete are not ticked again in subsequent iterations
