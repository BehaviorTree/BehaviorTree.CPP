/*  Copyright (C) 2018-2022 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <filesystem>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/utils/shared_library.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "wildcards/wildcards.hpp"

namespace BT
{

bool WildcardMatch(std::string const& str, StringView filter)
{
  return wildcards::match(str, filter);
}

struct BehaviorTreeFactory::PImpl
{
  std::unordered_map<std::string, NodeBuilder> builders;
  std::unordered_map<std::string, TreeNodeManifest> manifests;
  std::set<std::string> builtin_IDs;
  std::unordered_map<std::string, Any> behavior_tree_definitions;
  std::shared_ptr<std::unordered_map<std::string, int>> scripting_enums;
  std::shared_ptr<BT::Parser> parser;
  std::unordered_map<std::string, SubstitutionRule> substitution_rules;
};

BehaviorTreeFactory::BehaviorTreeFactory() : _p(new PImpl)
{
  _p->parser = std::make_shared<XMLParser>(*this);
  registerNodeType<FallbackNode>("Fallback");
  registerNodeType<FallbackNode>("AsyncFallback", true);
  registerNodeType<SequenceNode>("Sequence");
  registerNodeType<SequenceNode>("AsyncSequence", true);
  registerNodeType<SequenceWithMemory>("SequenceWithMemory");

#ifdef USE_BTCPP3_OLD_NAMES
  registerNodeType<SequenceWithMemory>("SequenceStar");  // backward compatibility
#endif

  registerNodeType<ParallelNode>("Parallel");
  registerNodeType<ParallelAllNode>("ParallelAll");
  registerNodeType<ReactiveSequence>("ReactiveSequence");
  registerNodeType<ReactiveFallback>("ReactiveFallback");
  registerNodeType<IfThenElseNode>("IfThenElse");
  registerNodeType<WhileDoElseNode>("WhileDoElse");

  registerNodeType<InverterNode>("Inverter");

  registerNodeType<RetryNode>("RetryUntilSuccessful");
  registerNodeType<KeepRunningUntilFailureNode>("KeepRunningUntilFailure");
  registerNodeType<RepeatNode>("Repeat");
  registerNodeType<TimeoutNode>("Timeout");
  registerNodeType<DelayNode>("Delay");
  registerNodeType<RunOnceNode>("RunOnce");

  registerNodeType<ForceSuccessNode>("ForceSuccess");
  registerNodeType<ForceFailureNode>("ForceFailure");

  registerNodeType<AlwaysSuccessNode>("AlwaysSuccess");
  registerNodeType<AlwaysFailureNode>("AlwaysFailure");
  registerNodeType<ScriptNode>("Script");
  registerNodeType<ScriptCondition>("ScriptCondition");
  registerNodeType<SetBlackboardNode>("SetBlackboard");
  registerNodeType<SleepNode>("Sleep");
  registerNodeType<UnsetBlackboardNode>("UnsetBlackboard");

  registerNodeType<SubTreeNode>("SubTree");

  registerNodeType<PreconditionNode>("Precondition");

  registerNodeType<SwitchNode<2>>("Switch2");
  registerNodeType<SwitchNode<3>>("Switch3");
  registerNodeType<SwitchNode<4>>("Switch4");
  registerNodeType<SwitchNode<5>>("Switch5");
  registerNodeType<SwitchNode<6>>("Switch6");

  registerNodeType<LoopNode<int>>("LoopInt");
  registerNodeType<LoopNode<bool>>("LoopBool");
  registerNodeType<LoopNode<double>>("LoopDouble");
  registerNodeType<LoopNode<std::string>>("LoopString");

  registerNodeType<EntryUpdatedAction>("WasEntryUpdated");
  registerNodeType<EntryUpdatedDecorator>("SkipUnlessUpdated", NodeStatus::SKIPPED);
  registerNodeType<EntryUpdatedDecorator>("WaitValueUpdate", NodeStatus::RUNNING);

  for(const auto& it : _p->builders)
  {
    _p->builtin_IDs.insert(it.first);
  }

  _p->scripting_enums = std::make_shared<std::unordered_map<std::string, int>>();
}

BehaviorTreeFactory::~BehaviorTreeFactory()
{}

bool BehaviorTreeFactory::unregisterBuilder(const std::string& ID)
{
  if(builtinNodes().count(ID))
  {
    throw LogicError("You can not remove the builtin registration ID [", ID, "]");
  }
  auto it = _p->builders.find(ID);
  if(it == _p->builders.end())
  {
    return false;
  }
  _p->builders.erase(ID);
  _p->manifests.erase(ID);
  return true;
}

void BehaviorTreeFactory::registerBuilder(const TreeNodeManifest& manifest,
                                          const NodeBuilder& builder)
{
  auto it = _p->builders.find(manifest.registration_ID);
  if(it != _p->builders.end())
  {
    throw BehaviorTreeException("ID [", manifest.registration_ID, "] already registered");
  }

  _p->builders.insert({ manifest.registration_ID, builder });
  _p->manifests.insert({ manifest.registration_ID, manifest });
}

void BehaviorTreeFactory::registerSimpleCondition(
    const std::string& ID, const SimpleConditionNode::TickFunctor& tick_functor,
    PortsList ports)
{
  NodeBuilder builder = [tick_functor, ID](const std::string& name,
                                           const NodeConfig& config) {
    return std::make_unique<SimpleConditionNode>(name, tick_functor, config);
  };

  TreeNodeManifest manifest = { NodeType::CONDITION, ID, std::move(ports), {} };
  registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerSimpleAction(
    const std::string& ID, const SimpleActionNode::TickFunctor& tick_functor,
    PortsList ports)
{
  NodeBuilder builder = [tick_functor, ID](const std::string& name,
                                           const NodeConfig& config) {
    return std::make_unique<SimpleActionNode>(name, tick_functor, config);
  };

  TreeNodeManifest manifest = { NodeType::ACTION, ID, std::move(ports), {} };
  registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerSimpleDecorator(
    const std::string& ID, const SimpleDecoratorNode::TickFunctor& tick_functor,
    PortsList ports)
{
  NodeBuilder builder = [tick_functor, ID](const std::string& name,
                                           const NodeConfig& config) {
    return std::make_unique<SimpleDecoratorNode>(name, tick_functor, config);
  };

  TreeNodeManifest manifest = { NodeType::DECORATOR, ID, std::move(ports), {} };
  registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerFromPlugin(const std::string& file_path)
{
  BT::SharedLibrary loader;
  loader.load(file_path);
  typedef void (*Func)(BehaviorTreeFactory&);

  if(loader.hasSymbol(PLUGIN_SYMBOL))
  {
    Func func = (Func)loader.getSymbol(PLUGIN_SYMBOL);
    func(*this);
  }
  else
  {
    std::cout << "ERROR loading library [" << file_path << "]: can't find symbol ["
              << PLUGIN_SYMBOL << "]" << std::endl;
  }
}

void BehaviorTreeFactory::registerFromROSPlugins()
{
  throw RuntimeError("Using attribute [ros_pkg] in <include>, but this library was "
                     "compiled without ROS support. Recompile the BehaviorTree.CPP "
                     "using catkin");
}

void BehaviorTreeFactory::registerBehaviorTreeFromFile(
    const std::filesystem::path& filename)
{
  _p->parser->loadFromFile(filename);
}

void BehaviorTreeFactory::registerBehaviorTreeFromText(const std::string& xml_text)
{
  _p->parser->loadFromText(xml_text);
}

std::vector<std::string> BehaviorTreeFactory::registeredBehaviorTrees() const
{
  return _p->parser->registeredBehaviorTrees();
}

void BehaviorTreeFactory::clearRegisteredBehaviorTrees()
{
  _p->parser->clearInternalState();
}

std::unique_ptr<TreeNode> BehaviorTreeFactory::instantiateTreeNode(
    const std::string& name, const std::string& ID, const NodeConfig& config) const
{
  auto idNotFound = [this, ID] {
    std::cerr << ID << " not included in this list:" << std::endl;
    for(const auto& builder_it : _p->builders)
    {
      std::cerr << builder_it.first << std::endl;
    }
    throw RuntimeError("BehaviorTreeFactory: ID [", ID, "] not registered");
  };

  auto it_manifest = _p->manifests.find(ID);
  if(it_manifest == _p->manifests.end())
  {
    idNotFound();
  }

  std::unique_ptr<TreeNode> node;

  bool substituted = false;
  for(const auto& [filter, rule] : _p->substitution_rules)
  {
    if(filter == name || filter == ID || wildcards::match(config.path, filter))
    {
      // first case: the rule is simply a string with the name of the
      // node to create instead
      if(const auto substituted_ID = std::get_if<std::string>(&rule))
      {
        auto it_builder = _p->builders.find(*substituted_ID);
        if(it_builder != _p->builders.end())
        {
          auto& builder = it_builder->second;
          node = builder(name, config);
        }
        else
        {
          throw RuntimeError("Substituted Node ID [", *substituted_ID, "] not found");
        }
        substituted = true;
        break;
      }
      else if(const auto test_config = std::get_if<TestNodeConfig>(&rule))
      {
        node = std::make_unique<TestNode>(name, config,
                                          std::make_shared<TestNodeConfig>(*test_config));
        substituted = true;
        break;
      }
      else if(const auto test_config =
                  std::get_if<std::shared_ptr<TestNodeConfig>>(&rule))
      {
        node = std::make_unique<TestNode>(name, config, *test_config);
        substituted = true;
        break;
      }
      else
      {
        throw LogicError("Substitution rule is not a string or a TestNodeConfig");
      }
    }
  }

  // No substitution rule applied: default behavior
  if(!substituted)
  {
    auto it_builder = _p->builders.find(ID);
    if(it_builder == _p->builders.end())
    {
      idNotFound();
    }
    auto& builder = it_builder->second;
    node = builder(name, config);
  }

  node->setRegistrationID(ID);
  node->config().enums = _p->scripting_enums;

  auto AssignConditions = [](auto& conditions, auto& executors) {
    for(const auto& [cond_id, script] : conditions)
    {
      if(auto executor = ParseScript(script))
      {
        executors[size_t(cond_id)] = executor.value();
      }
      else
      {
        throw LogicError("Error in the script \"", script, "\"\n", executor.error());
      }
    }
  };
  AssignConditions(config.pre_conditions, node->preConditionsScripts());
  AssignConditions(config.post_conditions, node->postConditionsScripts());

  return node;
}

const std::unordered_map<std::string, NodeBuilder>& BehaviorTreeFactory::builders() const
{
  return _p->builders;
}

const std::unordered_map<std::string, TreeNodeManifest>&
BehaviorTreeFactory::manifests() const
{
  return _p->manifests;
}

const std::set<std::string>& BehaviorTreeFactory::builtinNodes() const
{
  return _p->builtin_IDs;
}

Tree BehaviorTreeFactory::createTreeFromText(const std::string& text,
                                             Blackboard::Ptr blackboard)
{
  if(!_p->parser->registeredBehaviorTrees().empty())
  {
    std::cout << "WARNING: You executed BehaviorTreeFactory::createTreeFromText "
                 "after registerBehaviorTreeFrom[File/Text].\n"
                 "This is NOT, probably, what you want to do.\n"
                 "You should probably use BehaviorTreeFactory::createTree, instead"
              << std::endl;
  }
  XMLParser parser(*this);
  parser.loadFromText(text);
  auto tree = parser.instantiateTree(blackboard);
  tree.manifests = this->manifests();
  return tree;
}

Tree BehaviorTreeFactory::createTreeFromFile(const std::filesystem::path& file_path,
                                             Blackboard::Ptr blackboard)
{
  if(!_p->parser->registeredBehaviorTrees().empty())
  {
    std::cout << "WARNING: You executed BehaviorTreeFactory::createTreeFromFile "
                 "after registerBehaviorTreeFrom[File/Text].\n"
                 "This is NOT, probably, what you want to do.\n"
                 "You should probably use BehaviorTreeFactory::createTree, instead"
              << std::endl;
  }

  XMLParser parser(*this);
  parser.loadFromFile(file_path);
  auto tree = parser.instantiateTree(blackboard);
  tree.manifests = this->manifests();
  return tree;
}

Tree BehaviorTreeFactory::createTree(const std::string& tree_name,
                                     Blackboard::Ptr blackboard)
{
  auto tree = _p->parser->instantiateTree(blackboard, tree_name);
  tree.manifests = this->manifests();
  return tree;
}

void BehaviorTreeFactory::addMetadataToManifest(const std::string& node_id,
                                                const KeyValueVector& metadata)
{
  auto it = _p->manifests.find(node_id);
  if(it == _p->manifests.end())
  {
    throw std::runtime_error("addMetadataToManifest: wrong ID");
  }
  it->second.metadata = metadata;
}

void BehaviorTreeFactory::registerScriptingEnum(StringView name, int value)
{
  const auto str = std::string(name);
  auto it = _p->scripting_enums->find(str);
  if(it == _p->scripting_enums->end())
  {
    _p->scripting_enums->insert({ str, value });
  }
  else
  {
    if(it->second != value)
    {
      throw LogicError(
          StrCat("Registering the enum [", name, "] twice with different values, first ",
                 std::to_string(it->second), " and later ", std::to_string(value)));
    }
  }
}

void BehaviorTreeFactory::clearSubstitutionRules()
{
  _p->substitution_rules.clear();
}

void BehaviorTreeFactory::addSubstitutionRule(StringView filter, SubstitutionRule rule)
{
  _p->substitution_rules[std::string(filter)] = rule;
}

void BehaviorTreeFactory::loadSubstitutionRuleFromJSON(const std::string& json_text)
{
  auto const json = nlohmann::json::parse(json_text);

  std::unordered_map<std::string, TestNodeConfig> configs;

  auto test_configs = json.at("TestNodeConfigs");
  for(auto const& [name, test_config] : test_configs.items())
  {
    auto& config = configs[name];

    auto return_status = test_config.at("return_status").get<std::string>();
    config.return_status = convertFromString<NodeStatus>(return_status);
    if(test_config.contains("async_delay"))
    {
      config.async_delay =
          std::chrono::milliseconds(test_config["async_delay"].get<int>());
    }
    if(test_config.contains("post_script"))
    {
      config.post_script = test_config["post_script"].get<std::string>();
    }
    if(test_config.contains("success_script"))
    {
      config.success_script = test_config["success_script"].get<std::string>();
    }
    if(test_config.contains("failure_script"))
    {
      config.failure_script = test_config["failure_script"].get<std::string>();
    }
  }

  auto substitutions = json.at("SubstitutionRules");
  for(auto const& [node_name, test] : substitutions.items())
  {
    auto test_name = test.get<std::string>();
    auto it = configs.find(test_name);
    if(it == configs.end())
    {
      addSubstitutionRule(node_name, test_name);
    }
    else
    {
      addSubstitutionRule(node_name, it->second);
    }
  }
}

const std::unordered_map<std::string, BehaviorTreeFactory::SubstitutionRule>&
BehaviorTreeFactory::substitutionRules() const
{
  return _p->substitution_rules;
}

Tree::Tree()
{}

void Tree::initialize()
{
  wake_up_ = std::make_shared<WakeUpSignal>();
  for(auto& subtree : subtrees)
  {
    for(auto& node : subtree->nodes)
    {
      node->setWakeUpInstance(wake_up_);
    }
  }
}

void Tree::haltTree()
{
  if(!rootNode())
  {
    return;
  }
  // the halt should propagate to all the node if the nodes
  // have been implemented correctly
  rootNode()->haltNode();

  //but, just in case.... this should be no-op
  auto visitor = [](BT::TreeNode* node) { node->haltNode(); };
  BT::applyRecursiveVisitor(rootNode(), visitor);

  rootNode()->resetStatus();
}

TreeNode* Tree::rootNode() const
{
  if(subtrees.empty())
  {
    return nullptr;
  }
  auto& subtree_nodes = subtrees.front()->nodes;
  return subtree_nodes.empty() ? nullptr : subtree_nodes.front().get();
}

bool Tree::sleep(std::chrono::system_clock::duration timeout)
{
  return wake_up_->waitFor(
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
}

Tree::~Tree()
{
  haltTree();
}

NodeStatus Tree::tickExactlyOnce()
{
  return tickRoot(EXACTLY_ONCE, std::chrono::milliseconds(0));
}

NodeStatus Tree::tickOnce()
{
  return tickRoot(ONCE_UNLESS_WOKEN_UP, std::chrono::milliseconds(0));
}

NodeStatus Tree::tickWhileRunning(std::chrono::milliseconds sleep_time)
{
  return tickRoot(WHILE_RUNNING, sleep_time);
}

Blackboard::Ptr Tree::rootBlackboard()
{
  if(subtrees.size() > 0)
  {
    return subtrees.front()->blackboard;
  }
  return {};
}

void Tree::applyVisitor(const std::function<void(const TreeNode*)>& visitor) const
{
  BT::applyRecursiveVisitor(static_cast<const TreeNode*>(rootNode()), visitor);
}

void Tree::applyVisitor(const std::function<void(TreeNode*)>& visitor)
{
  BT::applyRecursiveVisitor(static_cast<TreeNode*>(rootNode()), visitor);
}

uint16_t Tree::getUID()
{
  auto uid = ++uid_counter_;
  return uid;
}

NodeStatus Tree::tickRoot(TickOption opt, std::chrono::milliseconds sleep_time)
{
  NodeStatus status = NodeStatus::IDLE;

  if(!wake_up_)
  {
    initialize();
  }

  if(!rootNode())
  {
    throw RuntimeError("Empty Tree");
  }

  while(status == NodeStatus::IDLE ||
        (opt == TickOption::WHILE_RUNNING && status == NodeStatus::RUNNING))
  {
    status = rootNode()->executeTick();

    // Inner loop. The previous tick might have triggered the wake-up
    // in this case, unless TickOption::EXACTLY_ONCE, we tick again
    while(opt != TickOption::EXACTLY_ONCE && status == NodeStatus::RUNNING &&
          wake_up_->waitFor(std::chrono::milliseconds(0)))
    {
      status = rootNode()->executeTick();
    }

    if(isStatusCompleted(status))
    {
      rootNode()->resetStatus();
    }
    if(status == NodeStatus::RUNNING && sleep_time.count() > 0)
    {
      sleep(std::chrono::milliseconds(sleep_time));
    }
  }

  return status;
}

void BlackboardRestore(const std::vector<Blackboard::Ptr>& backup, Tree& tree)
{
  assert(backup.size() == tree.subtrees.size());
  for(size_t i = 0; i < tree.subtrees.size(); i++)
  {
    backup[i]->cloneInto(*(tree.subtrees[i]->blackboard));
  }
}

std::vector<Blackboard::Ptr> BlackboardBackup(const Tree& tree)
{
  std::vector<Blackboard::Ptr> bb;
  bb.reserve(tree.subtrees.size());
  for(const auto& sub : tree.subtrees)
  {
    bb.push_back(BT::Blackboard::create());
    sub->blackboard->cloneInto(*bb.back());
  }
  return bb;
}

nlohmann::json ExportTreeToJSON(const Tree& tree)
{
  nlohmann::json out;
  for(const auto& subtree : tree.subtrees)
  {
    nlohmann::json json_sub;
    auto sub_name = subtree->instance_name;
    if(sub_name.empty())
    {
      sub_name = subtree->tree_ID;
    }
    out[sub_name] = ExportBlackboardToJSON(*subtree->blackboard);
  }
  return out;
}

void ImportTreeFromJSON(const nlohmann::json& json, Tree& tree)
{
  if(json.size() != tree.subtrees.size())
  {
    throw std::runtime_error("Number of blackboards don't match:");
  }

  size_t index = 0;
  for(auto& [key, array] : json.items())
  {
    auto& subtree = tree.subtrees.at(index++);
    ImportBlackboardFromJSON(array, *subtree->blackboard);
  }
}

}  // namespace BT
