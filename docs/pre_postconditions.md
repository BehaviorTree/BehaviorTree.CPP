# Pre-conditions and Post-conditions

This document describes the pre-condition and post-condition attributes that can be attached to any node in XML.

## Pre-conditions

Pre-conditions are evaluated **before** a node's `tick()` method is called. They can short-circuit the node execution by returning a status immediately.

### Available Pre-conditions

| Attribute | When Evaluated | Behavior |
|-----------|----------------|----------|
| `_failureIf` | IDLE only | If true, return FAILURE without calling tick() |
| `_successIf` | IDLE only | If true, return SUCCESS without calling tick() |
| `_skipIf` | IDLE only | If true, return SKIPPED without calling tick() |
| `_while` | IDLE and RUNNING | If false when IDLE, return SKIPPED. If false when RUNNING, halt node and return SKIPPED |

### Evaluation Order

When a node has multiple pre-conditions, they are evaluated in this order:
1. `_failureIf`
2. `_successIf`
3. `_skipIf`
4. `_while`

The first condition that triggers will determine the result.

### Important: One-time Evaluation

**`_failureIf`, `_successIf`, and `_skipIf` are evaluated only once** when the node transitions from IDLE (or SKIPPED) to another state. They are **NOT re-evaluated** while the node is RUNNING.

This means if you have:
```xml
<MyAction _successIf="condition"/>
```

The `condition` is checked only when `MyAction` starts. If `MyAction` returns RUNNING, subsequent ticks will continue executing `MyAction` without re-checking the condition.

### The `_while` Exception

`_while` is the only pre-condition that is re-evaluated on every tick, even while the node is RUNNING. If the condition becomes false while the node is running, the node is halted and returns SKIPPED.

```xml
<MyAction _while="battery_ok"/>
```

If `battery_ok` becomes false while `MyAction` is running, the action is interrupted.

### When to Use Each Pre-condition

- **`_skipIf`**: Skip a node without failing the parent (useful in Sequences)
- **`_failureIf`**: Fail early based on a condition (useful in Fallbacks)
- **`_successIf`**: Succeed early based on a condition
- **`_while`**: Guard that must remain true for the entire execution

### Re-evaluating Conditions Every Tick

If you need a condition to be checked on every tick (not just when transitioning from IDLE), use the `<Precondition>` decorator node instead of inline attributes:

```xml
<!-- This checks the condition on every tick while child is RUNNING -->
<Precondition if="my_condition" else="RUNNING">
  <MyAction/>
</Precondition>
```

With `else="RUNNING"`, if the condition is false, the decorator returns RUNNING (keeping the tree alive) rather than SUCCESS/FAILURE/SKIPPED.

## Post-conditions

Post-conditions are scripts executed **after** a node completes (or is halted).

### Available Post-conditions

| Attribute | When Executed |
|-----------|---------------|
| `_onSuccess` | After node returns SUCCESS |
| `_onFailure` | After node returns FAILURE |
| `_onHalted` | After node is halted (including by `_while`) |
| `_post` | After any completion (SUCCESS, FAILURE, or HALTED) |

### Example

```xml
<MyAction
    _onSuccess="result := 'ok'"
    _onFailure="result := 'failed'"
    _onHalted="result := 'interrupted'"/>
```

## Common Patterns

### Conditional Execution in Sequence

```xml
<Sequence>
  <CheckBattery/>
  <MoveToGoal _skipIf="already_at_goal"/>
  <PickObject/>
</Sequence>
```

If `already_at_goal` is true, `MoveToGoal` is skipped and the sequence continues with `PickObject`.

### Early Exit in Fallback

```xml
<Fallback>
  <CachedResult _successIf="cache_valid"/>
  <ComputeResult/>
</Fallback>
```

If `cache_valid` is true, `CachedResult` succeeds immediately without executing.

### Guarded Action

```xml
<MoveToGoal _while="battery_level > 20"/>
```

The movement continues only while battery is sufficient. If battery drops, the action is halted.

## Related

- [Port Connection Rules](PORT_CONNECTION_RULES.md) - How ports connect between nodes
- [Name Validation Rules](name_validation_rules.md) - Valid names for ports and nodes
