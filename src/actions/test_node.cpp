#include "behaviortree_cpp/actions/test_node.h"

void BT::TestNode::setConfig(const TestNodeConfig &config)
{
  if(config.return_status == NodeStatus::IDLE)
  {
    throw RuntimeError("TestNode can not return IDLE");
  }
  _test_config = config;

  if(!_test_config.post_script.empty())
  {
    auto executor = ParseScript(_test_config.post_script);
    if (!executor)
    {
      throw RuntimeError(executor.error());
    }
    _executor = executor.value();
  }
}

BT::NodeStatus BT::TestNode::onStart()
{
  if(_test_config.pre_func)
  {
    _test_config.pre_func();
  }

  if( _test_config.async_delay <= std::chrono::milliseconds(0) )
  {
    return onCompleted();
  }
  // convert this in an asynchronous operation. Use another thread to count
  // a certain amount of time.
  _completed = false;
  _timer.add(std::chrono::milliseconds(_test_config.async_delay),
             [this](bool aborted) {
               if(!aborted)
               {
                 _completed.store(true);
                 this->emitWakeUpSignal();
               }
               else {
                 _completed.store(false);
               }
             });
  return NodeStatus::RUNNING;
}

BT::NodeStatus BT::TestNode::onRunning()
{
  if(_completed) {
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
  if (_executor)
  {
    Ast::Environment env = {config().blackboard, config().enums};
    _executor(env);
  }
  if(_test_config.post_func)
  {
    _test_config.post_func();
  }
  return _test_config.return_status;
}
