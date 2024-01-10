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
#include <fstream>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/utils/shared_library.h"
#include "behaviortree_cpp/contrib/json.hpp"
#include "behaviortree_cpp/xml_parsing.h"
#include "wildcards/wildcards.hpp"

#ifdef USING_ROS
#include <ros/package.h>
#endif

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

BehaviorTreeFactory::BehaviorTreeFactory():
  _p(new PImpl)
{
  _p->parser = std::make_shared<XMLParser>(*this);
  registerNodeType<FallbackNode>("Fallback");
  registerNodeType<FallbackNode>("AsyncFallback", true);
  registerNodeType<SequenceNode>("Sequence");
  registerNodeType<SequenceNode>("AsyncSequence", true);
  registerNodeType<SequenceWithMemory>("SequenceWithMemory");

#ifdef USE_BTCPP3_OLD_NAMES
  registerNodeType<SequenceWithMemory>("SequenceStar");   // backward compatibility
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

  for (const auto& it : _p->builders)
  {
    _p->builtin_IDs.insert(it.first);
  }

  _p->scripting_enums = std::make_shared<std::unordered_map<std::string, int>>();
}

BehaviorTreeFactory::BehaviorTreeFactory(BehaviorTreeFactory &&other) noexcept
{
  this->_p = std::move(other._p);
}

BehaviorTreeFactory &BehaviorTreeFactory::operator=(BehaviorTreeFactory &&other) noexcept
{
  this->_p = std::move(other._p);
  return *this;
}

BehaviorTreeFactory::~BehaviorTreeFactory()
{}

bool BehaviorTreeFactory::unregisterBuilder(const std::string& ID)
{
  if (builtinNodes().count(ID))
  {
    throw LogicError("You can not remove the builtin registration ID [", ID, "]");
  }
  auto it = _p->builders.find(ID);
  if (it == _p->builders.end())
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
  if (it != _p->builders.end())
  {
    throw BehaviorTreeException("ID [", manifest.registration_ID, "] already registered");
  }

  _p->builders.insert({manifest.registration_ID, builder});
  _p->manifests.insert({manifest.registration_ID, manifest});
}

void BehaviorTreeFactory::registerSimpleCondition(
    const std::string& ID, const SimpleConditionNode::TickFunctor& tick_functor,
    PortsList ports)
{
  NodeBuilder builder = [tick_functor, ID](const std::string& name,
                                           const NodeConfig& config) {
    return std::make_unique<SimpleConditionNode>(name, tick_functor, config);
  };

  TreeNodeManifest manifest = {NodeType::CONDITION, ID, std::move(ports), {}};
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

  TreeNodeManifest manifest = {NodeType::ACTION, ID, std::move(ports), {}};
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

  TreeNodeManifest manifest = {NodeType::DECORATOR, ID, std::move(ports), {}};
  registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerFromPlugin(const std::string& file_path)
{
  BT::SharedLibrary loader;
  loader.load(file_path);
  typedef void (*Func)(BehaviorTreeFactory&);

  if (loader.hasSymbol(PLUGIN_SYMBOL))
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

#ifdef USING_ROS

#ifdef _WIN32
const char os_pathsep(';');   // NOLINT
#else
const char os_pathsep(':');   // NOLINT
#endif

// This function is a copy from the one in class_loader_imp.hpp in ROS pluginlib
// package, licensed under BSD.
// https://github.com/ros/pluginlib
std::vector<std::string> getCatkinLibraryPaths()
{
  std::vector<std::string> lib_paths;
  const char* env = std::getenv("CMAKE_PREFIX_PATH");
  if (env)
  {
    const std::string env_catkin_prefix_paths(env);
    std::vector<BT::StringView> catkin_prefix_paths =
        splitString(env_catkin_prefix_paths, os_pathsep);
    for (BT::StringView catkin_prefix_path : catkin_prefix_paths)
    {
      std::filesystem::path path(static_cast<std::string>(catkin_prefix_path));
      std::filesystem::path lib("lib");
      lib_paths.push_back((path / lib).string());
    }
  }
  return lib_paths;
}

void BehaviorTreeFactory::registerFromROSPlugins()
{
  std::vector<std::string> plugins;
  ros::package::getPlugins("behaviortree_cpp", "bt_lib_plugin", plugins, true);
  std::vector<std::string> catkin_lib_paths = getCatkinLibraryPaths();

  for (const auto& plugin : plugins)
  {
    auto filename = std::filesystem::path(plugin + BT::SharedLibrary::suffix());
    for (const auto& lib_path : catkin_lib_paths)
    {
      const auto full_path = std::filesystem::path(lib_path) / filename;
      if (std::filesystem::exists(full_path))
      {
        std::cout << "Registering ROS plugins from " << full_path.string() << std::endl;
        registerFromPlugin(full_path.string());
        break;
      }
    }
  }
}
#else

void BehaviorTreeFactory::registerFromROSPlugins()
{
  throw RuntimeError("Using attribute [ros_pkg] in <include>, but this library was "
                     "compiled without ROS support. Recompile the BehaviorTree.CPP "
                     "using catkin");
}
#endif

void BehaviorTreeFactory::registerBehaviorTreeFromFile(const std::filesystem::path &filename)
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
  auto idNotFound = [this, ID]
  {
    std::cerr << ID << " not included in this list:" << std::endl;
    for (const auto& builder_it : _p->builders)
    {
      std::cerr << builder_it.first << std::endl;
    }
    throw RuntimeError("BehaviorTreeFactory: ID [", ID, "] not registered");
  };

  auto it_manifest = _p->manifests.find(ID);
  if (it_manifest == _p->manifests.end())
  {
    idNotFound();
  }


  std::unique_ptr<TreeNode> node;

  bool substituted = false;
  for(const auto& [filter, rule]: _p->substitution_rules)
  {
    if( filter == name || filter == ID || wildcards::match(config.path, filter))
    {
      // first case: the rule is simply a string with the name of the
      // node to create instead
      if(const auto substituted_ID = std::get_if<std::string>(&rule) )
      {
        auto it_builder = _p->builders.find(*substituted_ID);
        if (it_builder != _p->builders.end())
        {
          auto& builder = it_builder->second;
          node = builder(name, config);
        }
        else{
          throw RuntimeError("Substituted Node ID not found");
        }
        substituted = true;
        break;
      }
      else if(const auto test_config = std::get_if<TestNodeConfig>(&rule) )
      {
        // second case, the varian is a TestNodeConfig
        auto test_node = new TestNode(name, config);
        test_node->setConfig(*test_config);

        node.reset(test_node);
        substituted = true;
        break;
      }
    }
  }

  // No substitution rule applied: default behavior
  if(!substituted)
  {
    auto it_builder = _p->builders.find(ID);
    if (it_builder == _p->builders.end())
    {
      idNotFound();
    }
    auto& builder = it_builder->second;
    node = builder(name, config);
  }

  node->setRegistrationID(ID);
  node->config().enums = _p->scripting_enums;

  auto AssignConditions = [](auto& conditions, auto& executors) {
    for (const auto& [cond_id, script] : conditions)
    {
      if (auto executor = ParseScript(script))
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
  if(!_p->parser->registeredBehaviorTrees().empty()) {
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

Tree BehaviorTreeFactory::createTreeFromFile(const std::filesystem::path &file_path,
                                             Blackboard::Ptr blackboard)
{
  if(!_p->parser->registeredBehaviorTrees().empty()) {
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
  if (it == _p->manifests.end())
  {
    throw std::runtime_error("addMetadataToManifest: wrong ID");
  }
  it->second.metadata = metadata;
}

void BehaviorTreeFactory::registerScriptingEnum(StringView name, int value)
{
  (*_p->scripting_enums)[std::string(name)] = value;
}

void BehaviorTreeFactory::clearSubstitutionRules()
{
  _p->substitution_rules.clear();
}

void BehaviorTreeFactory::addSubstitutionRule(StringView filter, SubstitutionRule rule)
{
  _p->substitution_rules[std::string(filter)] = rule;
}

void BehaviorTreeFactory::loadSubstitutionRuleFromJSON(const std::string &json_text)
{
  auto const json = nlohmann::json::parse(json_text);

  std::unordered_map<std::string, TestNodeConfig> configs;

  auto test_configs = json.at("TestNodeConfigs");
  for(auto const& [name, test_config]: test_configs.items())
  {
    auto& config = configs[name];

    auto status = test_config.at("return_status").get<std::string>();
    config.return_status = convertFromString<NodeStatus>(status);
    if(test_config.contains("async_delay"))
    {
      config.async_delay =
          std::chrono::milliseconds(test_config["async_delay"].get<int>());
    }
    if(test_config.contains("post_script"))
    {
      config.post_script = test_config["post_script"].get<std::string>();
    }
  }

  auto substitutions = json.at("SubstitutionRules");
  for(auto const& [node_name, test]: substitutions.items())
  {
    auto test_name = test.get<std::string>();
    auto it = configs.find(test_name);
    if(it == configs.end())
    {
      addSubstitutionRule(node_name, test_name);
    }
    else {
      addSubstitutionRule(node_name, it->second);
    }
  }
}

const std::unordered_map<std::string, BehaviorTreeFactory::SubstitutionRule> &
BehaviorTreeFactory::substitutionRules() const
{
  return _p->substitution_rules;
}


Tree &Tree::operator=(Tree &&other)
{
  subtrees = std::move(other.subtrees);
  manifests = std::move(other.manifests);
  wake_up_ = other.wake_up_;
  return *this;
}

Tree::Tree()
{}

Tree::Tree(Tree &&other)
{
  (*this) = std::move(other);
}

void Tree::initialize()
{
  wake_up_ = std::make_shared<WakeUpSignal>();
  for (auto& subtree : subtrees)
  {
    for (auto& node : subtree->nodes)
    {
      node->setWakeUpInstance(wake_up_);
    }
  }
}

void Tree::haltTree()
{
  if (!rootNode())
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
  if (subtrees.empty())
  {
    return nullptr;
  }
  auto& subtree_nodes = subtrees.front()->nodes;
  return subtree_nodes.empty() ? nullptr : subtree_nodes.front().get();
}

void Tree::sleep(std::chrono::system_clock::duration timeout)
{
  wake_up_->waitFor(std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
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
  if (subtrees.size() > 0)
  {
    return subtrees.front()->blackboard;
  }
  return {};
}

void Tree::applyVisitor(const std::function<void(const TreeNode*)>& visitor)
{
  for (auto const& subtree : subtrees)
  {
    BT::applyRecursiveVisitor(static_cast<const TreeNode*>(subtree->nodes.front().get()),
                              visitor);
  }
}

void Tree::applyVisitor(const std::function<void(TreeNode*)>& visitor)
{
  for (auto const& subtree : subtrees)
  {
    BT::applyRecursiveVisitor(static_cast<TreeNode*>(subtree->nodes.front().get()),
                              visitor);
  }
}

uint16_t Tree::getUID() {
  auto uid =  ++uid_counter_;
  return uid;
}

NodeStatus Tree::tickRoot(TickOption opt, std::chrono::milliseconds sleep_time)
{
  NodeStatus status = NodeStatus::IDLE;

  if (!wake_up_)
  {
    initialize();
  }

  if (!rootNode())
  {
    throw RuntimeError("Empty Tree");
  }

  while (status == NodeStatus::IDLE ||
         (opt == TickOption::WHILE_RUNNING && status == NodeStatus::RUNNING))
  {
    status = rootNode()->executeTick();

    // Inner loop. The previous tick might have triggered the wake-up
    // in this case, unless TickOption::EXACTLY_ONCE, we tick again
    while( opt != TickOption::EXACTLY_ONCE &&
           status == NodeStatus::RUNNING &&
           wake_up_->waitFor(std::chrono::milliseconds(0)) )
    {
      status = rootNode()->executeTick();
    }

    if (isStatusCompleted(status))
    {
      rootNode()->resetStatus();
    }
    if (status == NodeStatus::RUNNING && sleep_time.count() > 0)
    {
      sleep(std::chrono::milliseconds(sleep_time));
    }
  }

  return status;
}

}   // namespace BT
