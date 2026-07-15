#include "behaviortree_cpp/actions/test_node.h"

#include "behaviortree_cpp/scripting/script_parser.hpp"
#include "behaviortree_cpp/utils/timer_queue.h"

#include <atomic>

namespace BT
{

namespace
{

bool isAllowedTestNodeStatus(NodeStatus status)
{
  switch(status)
  {
    case NodeStatus::RUNNING:
    case NodeStatus::SUCCESS:
    case NodeStatus::FAILURE:
    case NodeStatus::SKIPPED:
      return true;

    case NodeStatus::IDLE:
      return false;
  }
  return false;
}

void validateTestNodeStatus(NodeStatus status, StringView source)
{
  if(!isAllowedTestNodeStatus(status))
  {
    throw RuntimeError("TestNode ", std::string(source),
                       " resolved to invalid status (integer value: ",
                       std::to_string(static_cast<int>(status)), ")");
  }
}

EnumsTablePtr createTestNodeEnums(const EnumsTablePtr& base_enums)
{
  auto enums = base_enums ? std::make_shared<EnumsTable>(*base_enums) :
                            std::make_shared<EnumsTable>();

  (*enums)["IDLE"] = static_cast<int>(NodeStatus::IDLE);
  (*enums)["RUNNING"] = static_cast<int>(NodeStatus::RUNNING);
  (*enums)["SUCCESS"] = static_cast<int>(NodeStatus::SUCCESS);
  (*enums)["FAILURE"] = static_cast<int>(NodeStatus::FAILURE);
  (*enums)["SKIPPED"] = static_cast<int>(NodeStatus::SKIPPED);
  return enums;
}

NodeStatus convertScriptResultToStatus(const Any& result)
{
  if(result.empty())
  {
    throw RuntimeError("return_status_script returned an empty value");
  }

  if(result.isString())
  {
    auto status = convertFromString<NodeStatus>(result.cast<std::string>());
    validateTestNodeStatus(status, "return_status_script");
    return status;
  }

  if(auto status = result.tryCast<int>())
  {
    auto resolved = static_cast<NodeStatus>(status.value());
    validateTestNodeStatus(resolved, "return_status_script");
    return resolved;
  }

  throw RuntimeError("return_status_script must evaluate to a NodeStatus-compatible "
                     "value");
}

}  // namespace

struct TestNode::PImpl
{
  std::shared_ptr<TestNodeConfig> config;
  ScriptFunction return_status_executor;
  ScriptFunction success_executor;
  ScriptFunction failure_executor;
  ScriptFunction post_executor;
  TimerQueue<> timer;
  std::atomic_bool completed = false;
};

TestNode::TestNode(const std::string& name, const NodeConfig& config,
                   TestNodeConfig test_config)
  : TestNode(name, config, std::make_shared<TestNodeConfig>(std::move(test_config)))
{}

TestNode::TestNode(const std::string& name, const NodeConfig& config,
                   std::shared_ptr<TestNodeConfig> test_config)
  : StatefulActionNode(name, config), _p(std::make_unique<PImpl>())
{
  _p->config = std::move(test_config);
  setRegistrationID("TestNode");

  if(_p->config->return_status_script.empty())
  {
    validateTestNodeStatus(_p->config->return_status, "return_status");
  }

  auto prepareScript = [](const std::string& script, auto& executor) {
    if(!script.empty())
    {
      auto result = ParseScript(script);
      if(!result)
      {
        throw RuntimeError(result.error());
      }
      executor = result.value();
    }
  };
  prepareScript(_p->config->return_status_script, _p->return_status_executor);
  prepareScript(_p->config->success_script, _p->success_executor);
  prepareScript(_p->config->failure_script, _p->failure_executor);
  prepareScript(_p->config->post_script, _p->post_executor);
}

TestNode::~TestNode() = default;

NodeStatus TestNode::onStart()
{
  if(_p->config->async_delay <= std::chrono::milliseconds(0))
  {
    return onCompleted();
  }
  // convert this in an asynchronous operation. Use another thread to count
  // a certain amount of time.
  _p->completed = false;
  _p->timer.add(std::chrono::milliseconds(_p->config->async_delay), [this](bool aborted) {
    if(!aborted)
    {
      _p->completed.store(true);
      this->emitWakeUpSignal();
    }
    else
    {
      _p->completed.store(false);
    }
  });
  return NodeStatus::RUNNING;
}

NodeStatus TestNode::onRunning()
{
  if(_p->completed)
  {
    return onCompleted();
  }
  return NodeStatus::RUNNING;
}

void TestNode::onHalted()
{
  _p->timer.cancelAll();
}

NodeStatus TestNode::onCompleted()
{
  NodeStatus status = NodeStatus::IDLE;

  if(_p->config->complete_func)
  {
    status = _p->config->complete_func();
  }
  else if(_p->return_status_executor)
  {
    // return_status_script may reference NodeStatus names (e.g. SUCCESS, FAILURE),
    // so it is evaluated with an enum table augmented with those values. The
    // post-execution scripts below deliberately do NOT see these names.
    auto status_enums = createTestNodeEnums(config().enums);
    Ast::Environment status_env = { config().blackboard, status_enums };
    status = convertScriptResultToStatus(_p->return_status_executor(status_env));
  }
  else
  {
    status = _p->config->return_status;
  }

  validateTestNodeStatus(status, "completion");

  // success/failure/post scripts use only the node's own enums, so that a
  // blackboard entry named like a status (e.g. "SUCCESS") is not shadowed.
  Ast::Environment env = { config().blackboard, config().enums };

  if(status == NodeStatus::SUCCESS && _p->success_executor)
  {
    _p->success_executor(env);
  }
  else if(status == NodeStatus::FAILURE && _p->failure_executor)
  {
    _p->failure_executor(env);
  }
  if(_p->post_executor)
  {
    _p->post_executor(env);
  }
  return status;
}

}  // namespace BT
