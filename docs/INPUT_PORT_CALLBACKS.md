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

Subscriptions bind to the resolved key at registration time. Reconfiguring port
or subtree remapping requires a new subscription. Subscriptions survive removing
and recreating that key. Removal itself does not notify.

Writes through `Blackboard::set()`, `TreeNode::setOutput()`, scripting assignments,
JSON import, and `Blackboard::cloneInto()` notify. Clone notifications run after
the entire snapshot has been copied, allowing a callback to read other restored
entries. Raw writes through `Entry`, `AnyPtrLocked`, or shared objects are not
observable automatically. Call `Blackboard::notifyEntryUpdated()` after releasing
the entry lock to explicitly publish such a write.

The lower-level `Blackboard::subscribeToEntry()` supports subscriptions before
the first assignment without creating or changing the type of an entry.

## Compatibility

This adds an opt-in API; existing port declarations and unsubscribed nodes keep
their behavior. The additional subscription storage changes `Blackboard`'s
binary layout. Rebuild the library and clients together; this API is proposed
for the next major version, as requested by issue #872's BT.CPP-5 label.
