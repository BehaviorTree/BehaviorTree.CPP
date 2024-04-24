#include "behaviortree_cpp/actions/test_node.h"

void BT::TestNode::setConfig(const TestNodeConfig& config)
{
  if(config.return_status == NodeStatus::IDLE)
  {
    throw RuntimeError("TestNode can not return IDLE");
  }
  _test_config = config;

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
  prepareScript(config.success_script, _success_executor);
  prepareScript(config.failure_script, _failure_executor);
  prepareScript(config.post_script, _post_executor);
}

BT::NodeStatus BT::TestNode::onStart()
{
  if(_test_config.async_delay <= std::chrono::milliseconds(0))
  {
    return onCompleted();
  }
  // convert this in an asynchronous operation. Use another thread to count
  // a certain amount of time.
  _completed = false;
  _timer.add(std::chrono::milliseconds(_test_config.async_delay), [this](bool aborted) {
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

BT::NodeStatus BT::TestNode::onRunning()
{
  if(_completed)
  {
    return onCompleted();
  }
  return NodeStatus::RUNNING;
}

void BT::TestNode::onHalted()
{
  _timer.cancelAll();
}

BT::NodeStatus BT::TestNode::onCompleted()
{
  Ast::Environment env = { config().blackboard, config().enums };

  auto status = _test_config.complete_func();
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
