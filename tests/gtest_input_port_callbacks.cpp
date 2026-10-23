#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/scripting/operators.hpp"

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

using namespace BT;

namespace
{
class InputObserver : public SyncActionNode
{
public:
  InputObserver(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  static PortsList providedPorts()
  {
    return { InputPort<int>("value"), InputPort<int>("constant", 42, "constant"),
             InputPort<int>("default_key", "{source}", "remapped default"),
             BidirectionalPort<int>("both"), OutputPort<int>("out") };
  }

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }
};
}  // namespace

TEST(InputPortCallbacks, NotifiesBeforeTickAndCanReadAndWrite)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<InputObserver>("Observer");
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree ID="Main">
      <Observer value="{source}"/>
    </BehaviorTree></root>)");
  auto* node = tree.rootNode();
  auto bb = tree.rootBlackboard();
  std::vector<int> values;
  auto subscription = node->subscribeToInputPort("value", [&] {
    values.push_back(node->getInput<int>("value").value());
    bb->set("other", values.back());
  });

  EXPECT_TRUE(values.empty());
  bb->set("source", 3);
  bb->set("source", 3);
  bb->set("source", 7);
  EXPECT_EQ(values, (std::vector<int>{ 3, 3, 7 }));
  EXPECT_EQ(bb->get<int>("other"), 7);
  EXPECT_EQ(node->status(), NodeStatus::IDLE);
  subscription.reset();
  bb->set("source", 9);
  EXPECT_EQ(values.size(), 3);
}

TEST(InputPortCallbacks, RemappingAndPrivateKeys)
{
  auto root = Blackboard::create();
  auto child = Blackboard::create(root);
  child->addSubtreeRemapping("local", "remote");
  child->addSubtreeRemapping("global", "@root_value");
  child->enableAutoRemapping(true);
  int manual = 0;
  int automatic = 0;
  int global = 0;
  int private_key = 0;
  auto a = child->subscribeToEntry("local", [&] { ++manual; });
  auto b = child->subscribeToEntry("auto_key", [&] { ++automatic; });
  auto c = child->subscribeToEntry("global", [&] { ++global; });
  auto d = child->subscribeToEntry("_private", [&] { ++private_key; });
  root->set("remote", 1);
  child->set("local", 2);
  root->set("auto_key", 1);
  child->set("auto_key", 2);
  root->set("root_value", 1);
  child->set("global", 2);
  root->set("_private", 1);
  EXPECT_EQ(private_key, 0);
  child->set("_private", 2);
  EXPECT_EQ(manual, 2);
  EXPECT_EQ(automatic, 2);
  EXPECT_EQ(global, 2);
  EXPECT_EQ(private_key, 1);
}

TEST(InputPortCallbacks, SubtreePort)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<InputObserver>("Observer");
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4" main_tree_to_execute="Main">
      <BehaviorTree ID="Main"><SubTree ID="Child" input="{source}"/></BehaviorTree>
      <BehaviorTree ID="Child"><Observer value="{input}"/></BehaviorTree>
    </root>)");
  auto* node = tree.subtrees.at(1)->nodes.front().get();
  int observed = 0;
  auto subscription = node->subscribeToInputPort(
      "value", [&] { observed = node->getInput<int>("value").value(); });
  tree.rootBlackboard()->set("source", 17);
  EXPECT_EQ(observed, 17);
}

TEST(InputPortCallbacks, DefaultsConstantsIdentityAndValidation)
{
  NodeConfig config;
  config.blackboard = Blackboard::create();
  TreeNodeManifest manifest = {
    NodeType::ACTION, "Observer", InputObserver::providedPorts(), {}
  };
  config.manifest = &manifest;
  config.input_ports["value"] = "{=}";
  config.input_ports["both"] = "{@root_key}";
  InputObserver node("observer", config);
  int count = 0;
  auto callback = [&] { ++count; };
  auto a = node.subscribeToInputPort("value", callback);
  auto b = node.subscribeToInputPort("default_key", callback);
  auto c = node.subscribeToInputPort("both", callback);
  EXPECT_FALSE(node.subscribeToInputPort("constant", callback));
  config.blackboard->set("value", 1);
  config.blackboard->set("source", 2);
  config.blackboard->set("root_key", 3);
  EXPECT_EQ(count, 3);
  EXPECT_THROW((void)node.subscribeToInputPort("missing", callback), LogicError);
  EXPECT_THROW((void)node.subscribeToInputPort("out", callback), LogicError);
  EXPECT_THROW((void)node.subscribeToInputPort("value", {}), LogicError);

  config.input_ports["value"] = "12";
  InputObserver literal("literal", config);
  EXPECT_FALSE(literal.subscribeToInputPort("value", callback));
  config.input_ports.erase("value");
  InputObserver missing("missing", config);
  EXPECT_THROW((void)missing.subscribeToInputPort("value", callback), LogicError);
  config.input_ports["value"] = "{source}";
  config.blackboard.reset();
  InputObserver no_blackboard("no_blackboard", config);
  EXPECT_THROW((void)no_blackboard.subscribeToInputPort("value", callback), LogicError);
}

TEST(InputPortCallbacks, FirstWriteUnsetAndFailedAssignment)
{
  auto bb = Blackboard::create();
  int count = 0;
  auto subscription = bb->subscribeToEntry("key", [&] {
    EXPECT_TRUE(bb->getEntry("key"));
    ++count;
  });
  EXPECT_FALSE(bb->getEntry("key"));
  bb->set("key", 1);
  EXPECT_THROW(bb->set("key", std::string("not a number")), RuntimeError);
  EXPECT_EQ(count, 1);
  bb->unset("key");
  bb->set("key", 2);
  EXPECT_EQ(count, 2);
  EXPECT_THROW((void)bb->subscribeToEntry("key", {}), LogicError);
}

TEST(InputPortCallbacks, ScriptsAndOutputPorts)
{
  auto bb = Blackboard::create();
  NodeConfig config;
  config.blackboard = bb;
  config.output_ports["out"] = "{key}";
  InputObserver writer("writer", config);
  std::vector<int> values;
  auto subscription =
      bb->subscribeToEntry("key", [&] { values.push_back(bb->get<int>("key")); });
  Ast::Environment environment = { bb, {} };
  EXPECT_TRUE(ParseScriptAndExecute(environment, "key := 1"));
  EXPECT_TRUE(ParseScriptAndExecute(environment, "key = 2; key += 3; key *= 2"));
  EXPECT_TRUE(writer.setOutput("out", 12));
  EXPECT_EQ(values, (std::vector<int>{ 1, 2, 5, 10, 12 }));
}

TEST(InputPortCallbacks, ImportAndRestore)
{
  auto bb = Blackboard::create();
  auto backup = Blackboard::create();
  backup->set("key", 4);
  backup->set("other", 8);
  int observed = 0;
  auto subscription =
      bb->subscribeToEntry("key", [&] { observed = bb->get<int>("key"); });
  ImportBlackboardFromJSON(nlohmann::json{ { "key", 2 } }, *bb);
  EXPECT_EQ(observed, 2);
  auto restore_subscription =
      bb->subscribeToEntry("key", [&] { EXPECT_EQ(bb->get<int>("other"), 8); });
  backup->cloneInto(*bb);
  EXPECT_EQ(observed, 4);
  restore_subscription.reset();
  bb->unset("key");
  backup->set("key", 6);
  backup->cloneInto(*bb);
  EXPECT_EQ(observed, 6);
}

TEST(InputPortCallbacks, CallbackExceptionLeavesValueCommitted)
{
  auto bb = Blackboard::create();
  auto subscription = bb->subscribeToEntry("key", [] { throw RuntimeError("callback"); });
  EXPECT_THROW(bb->set("key", 42), RuntimeError);
  EXPECT_EQ(bb->get<int>("key"), 42);
  subscription.reset();
  EXPECT_NO_THROW(bb->set("key", 43));
}

TEST(InputPortCallbacks, ReentrantSubscriptions)
{
  auto bb = Blackboard::create();
  int first_count = 0;
  int second_count = 0;
  Blackboard::EntryUpdatedSubscriber first;
  Blackboard::EntryUpdatedSubscriber second;
  first = bb->subscribeToEntry("key", [&] {
    ++first_count;
    first.reset();
    second = bb->subscribeToEntry("key", [&] { ++second_count; });
    bb->set("key", 2);
  });
  bb->set("key", 1);
  EXPECT_EQ(first_count, 1);
  EXPECT_EQ(second_count, 1);
  bb->set("key", 3);
  EXPECT_EQ(second_count, 2);
}

TEST(InputPortCallbacks, ConcurrentWritersAndSubscriptions)
{
  auto bb = Blackboard::create();
  bb->set("key", 0);
  std::atomic<int> count = 0;
  auto subscription = bb->subscribeToEntry("key", [&] {
    (void)bb->get<int>("key");
    ++count;
  });
  std::vector<std::thread> writers;
  for(int i = 0; i < 4; ++i)
  {
    writers.emplace_back([&] {
      for(int j = 0; j < 200; ++j)
      {
        auto temporary = bb->subscribeToEntry("key", [] {});
        bb->set("key", j);
      }
    });
  }
  for(auto& writer : writers)
  {
    writer.join();
  }
  EXPECT_EQ(count, 800);
}

TEST(InputPortCallbacks, BlackboardMayBeDestroyedBeforeHandle)
{
  auto bb = Blackboard::create();
  auto subscription = bb->subscribeToEntry("key", [] {});
  bb.reset();
  EXPECT_NO_THROW(subscription.reset());
}

TEST(InputPortCallbacks, LocalEntryTakesPrecedence)
{
  auto root = Blackboard::create();
  auto child = Blackboard::create(root);
  child->set("key", 1);
  child->enableAutoRemapping(true);
  int count = 0;
  auto subscription = child->subscribeToEntry("key", [&] { ++count; });
  root->set("key", 2);
  EXPECT_EQ(count, 0);
  child->set("key", 3);
  EXPECT_EQ(count, 1);
}

TEST(InputPortCallbacks, DirectMutationRequiresExplicitNotification)
{
  auto bb = Blackboard::create();
  bb->set("key", 1);
  int observed = 0;
  auto subscription =
      bb->subscribeToEntry("key", [&] { observed = bb->get<int>("key"); });
  {
    auto locked = bb->getAnyLocked("key");
    locked.assign(2);
  }
  EXPECT_EQ(observed, 0);
  bb->notifyEntryUpdated("key");
  EXPECT_EQ(observed, 2);
}

TEST(InputPortCallbacks, CallbackCanDisconnectLaterSubscriber)
{
  auto bb = Blackboard::create();
  Blackboard::EntryUpdatedSubscriber second;
  auto first = bb->subscribeToEntry("key", [&] { second.reset(); });
  second = bb->subscribeToEntry("key", [] { FAIL() << "Disconnected callback"; });
  bb->set("key", 1);
}

TEST(InputPortCallbacks, StatefulCallbackIsNotCopiedOnEachWrite)
{
  auto bb = Blackboard::create();
  int observed = 0;
  auto subscription =
      bb->subscribeToEntry("key", [&, count = 0]() mutable { observed = ++count; });
  bb->set("key", 1);
  bb->set("key", 2);
  EXPECT_EQ(observed, 2);
}
