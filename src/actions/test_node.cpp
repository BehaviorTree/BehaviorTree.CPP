#include "behaviortree_cpp/actions/test_node.h"

namespace BT
{

TestNode::TestNode(const std::string& name, const NodeConfig& config,
                   TestNodeConfig test_config)
  : TestNode(name, config, std::make_shared<TestNodeConfig>(std::move(test_config)))
{}

TestNode::TestNode(const std::string& name, const NodeConfig& config,
                   std::shared_ptr<TestNodeConfig> test_config)
  : StatefulActionNode(name, config), _config(std::move(test_config))
{
  setRegistrationID("TestNode");

  if(_config->return_status == NodeStatus::IDLE)
  {
    throw RuntimeError("TestNode can not return IDLE");
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
  Ast::Environment env = { config().blackboard, config().enums };

  auto status =
      (_config->complete_func) ? _config->complete_func() : _config->return_status;

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
