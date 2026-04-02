#include "behaviortree_cpp/actions/test_node.h"

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
                       " resolved to invalid status IDLE");
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

TestNode::TestNode(const std::string& name, const NodeConfig& config,
                   TestNodeConfig test_config)
  : TestNode(name, config, std::make_shared<TestNodeConfig>(std::move(test_config)))
{}

TestNode::TestNode(const std::string& name, const NodeConfig& config,
                   std::shared_ptr<TestNodeConfig> test_config)
  : StatefulActionNode(name, config), _config(std::move(test_config))
{
  setRegistrationID("TestNode");

  if(_config->return_status)
  {
    validateTestNodeStatus(_config->return_status.value(), "return_status");
  }

  if(!_config->complete_func && !_config->return_status &&
     _config->return_status_script.empty())
  {
    throw RuntimeError("TestNode requires one of complete_func, return_status, or "
                       "return_status_script");
  }

  _script_enums = createTestNodeEnums(config.enums);

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
  prepareScript(_config->return_status_script, _return_status_executor);
  prepareScript(_config->success_script, _success_executor);
  prepareScript(_config->failure_script, _failure_executor);
  prepareScript(_config->post_script, _post_executor);
}

NodeStatus TestNode::onStart()
{
  if(_config->async_delay <= std::chrono::milliseconds(0))
  {
    return onCompleted();
  }
  // convert this in an asynchronous operation. Use another thread to count
  // a certain amount of time.
  _completed = false;
  _timer.add(std::chrono::milliseconds(_config->async_delay), [this](bool aborted) {
    if(!aborted)
    {
      _completed.store(true);
      this->emitWakeUpSignal();
    }
    else
    {
      _completed.store(false);
    }
  });
  return NodeStatus::RUNNING;
}

NodeStatus TestNode::onRunning()
{
  if(_completed)
  {
    return onCompleted();
  }
  return NodeStatus::RUNNING;
}

void TestNode::onHalted()
{
  _timer.cancelAll();
}

NodeStatus TestNode::onCompleted()
{
  Ast::Environment env = { config().blackboard, _script_enums };

  NodeStatus status = NodeStatus::IDLE;

  if(_config->complete_func)
  {
    status = _config->complete_func();
  }
  else if(_return_status_executor)
  {
    status = convertScriptResultToStatus(_return_status_executor(env));
  }
  else if(_config->return_status)
  {
    status = _config->return_status.value();
  }
  else
  {
    throw RuntimeError("TestNode requires one of complete_func, return_status, or "
                       "return_status_script");
  }

  validateTestNodeStatus(status, "completion");

  if(status == NodeStatus::SUCCESS && _success_executor)
  {
    _success_executor(env);
  }
  else if(status == NodeStatus::FAILURE && _failure_executor)
  {
    _failure_executor(env);
  }
  if(_post_executor)
  {
    _post_executor(env);
  }
  return status;
}

}  // namespace BT
