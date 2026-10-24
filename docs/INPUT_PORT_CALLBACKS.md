# Input port callbacks

`TreeNode::subscribeToInputPort()` observes writes to the blackboard key backing
an input port. It is an instance method, so a callback can use node-specific
state. Port declarations in `providedPorts()` remain unchanged.

For example, a node can react to a new topic name before its next tick:

```cpp
class TopicListener : public BT::SyncActionNode
{
public:
  TopicListener(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {
    topic_subscription_ = subscribeToInputPort("topic", [this] {
      if(auto topic = getInput<std::string>("topic"))
      {
        configureSubscription(topic.value());
      }
    });
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("topic") };
  }

  BT::NodeStatus tick() override;

private:
  void configureSubscription(const std::string& topic);
  BT::Blackboard::EntryUpdatedSubscriber topic_subscription_;
};
```

The application must stop writers and wait for callbacks to finish before
destroying the node. Resetting the handle alone is not a synchronization barrier.
The example assumes the application serializes writes and access to the node's
subscription state.

## Notification contract

- Retain the returned handle to keep the callback registered. Resetting or
  destroying it unsubscribes; a callback already selected for invocation may
  still complete.
- Registration does not invoke the callback. Initialize from `getInput()`
  separately when an initial value is needed. With concurrent writers, coordinate
  initialization with the application's synchronization.
- Successful writes notify even if the new value equals the previous value.
  Generic port values do not necessarily have an equality operation.
- Callbacks run synchronously on the writer's thread, outside blackboard locks.
  They may call `getInput()`, write other keys, and change subscriptions.
  Writing the observed key can recursively invoke callbacks.
- Concurrent writes may invoke the same callback concurrently. A callback reads
  the latest value, not a snapshot of the triggering write; delivery order is
  not guaranteed across writers.
- Callback exceptions propagate after the write has committed. As with other
  synchronous signals, a throwing callback prevents the remaining callbacks in
  that notification from running.

## Supported inputs and writes

Explicit subtree remapping, automatic remapping, `{=}`, `{@root_key}`, and
blackboard-backed port defaults are supported. A literal or constant default
returns an empty handle because it cannot change through the blackboard.
Missing inputs, output-only ports, invalid blackboards, and empty callbacks are
reported with `LogicError`.

Subscriptions bind to the blackboard entry the key resolves to at registration
time. Reconfiguring port or subtree remapping requires a new subscription.
A subscription lives in the entry: `unset()` (or `cloneInto()` removing the key)
drops it, without notifying, and an entry re-created afterwards under the same
name is not observed. Subscribe again after re-creating the key.

If the entry doesn't exist yet, subscribing creates it, so the key is listed by
`getKeyNames()` before its first assignment, with an empty value.
`subscribeToInputPort()` creates it with the port's type, as tree creation does
for remapped ports. `Blackboard::subscribeToEntry()` creates a generic
(`AnyTypeAllowed`) entry, like `set<std::string>()` does: the first strongly typed
write sets its type. Note that, as for any existing generic entry, a first write
of a `std::string` locks such an entry to `std::string`, whereas without the
subscription `set<std::string>()` would have created a generic entry.

Writes through `Blackboard::set()`, `TreeNode::setOutput()`, scripting assignments,
JSON import, and `Blackboard::cloneInto()` notify. Clone notifications run after
the entire snapshot has been copied, allowing a callback to read other restored
entries. Raw writes through `Entry`, `AnyPtrLocked`, or shared objects are not
observable automatically. Call `Blackboard::notifyEntryUpdated()` after releasing
the entry lock to explicitly publish such a write.

## Compatibility

This adds an opt-in API; existing port declarations and unsubscribed nodes keep
their behavior. It is ABI compatible with 4.x: the subscribers are stored in a
member appended at the end of `Blackboard::Entry`, which is allocated only inside
the library, and the layout of `Blackboard` is unchanged. Without subscribers, a
write only checks that member for null under the entry lock it already holds.

Every write path (`set()`, `setOutput()`, scripting assignments) is inline code
in the headers. Writes performed by binaries compiled against older headers
(4.10 and earlier) store the value but do not notify, until they are rebuilt.
