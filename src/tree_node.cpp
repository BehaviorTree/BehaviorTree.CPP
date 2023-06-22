/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2022 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/tree_node.h"
#include <cstring>

namespace BT
{

struct TreeNode::PImpl
{
  PImpl(std::string name, NodeConfig config):
    name(std::move(name)),
    config(std::move(config))
  {}

  const std::string name;

  NodeStatus status = NodeStatus::IDLE;

  std::condition_variable state_condition_variable;

  mutable std::mutex state_mutex;

  StatusChangeSignal state_change_signal;

  NodeConfig config;

  std::string registration_ID;

  PreTickCallback substitution_callback;

  PostTickCallback post_condition_callback;

  std::mutex callback_injection_mutex;

  std::shared_ptr<WakeUpSignal> wake_up;

  std::array<ScriptFunction, size_t(PreCond::COUNT_)> pre_parsed;
  std::array<ScriptFunction, size_t(PostCond::COUNT_)> post_parsed;
};


TreeNode::TreeNode(std::string name, NodeConfig config) :
  _p(new PImpl(std::move(name), std::move(config)))
{
}

TreeNode::~TreeNode() {}

NodeStatus TreeNode::executeTick()
{
  auto new_status = _p->status;

  // a pre-condition may return the new status.
  // In this case it override the actual tick()
  if (auto precond = checkPreConditions())
  {
    new_status = precond.value();
  }
  else
  {
    // injected pre-callback
    bool substituted = false;
    if(!isStatusCompleted(_p->status))
    {
      PreTickCallback  callback;
      {
        std::unique_lock lk(_p->callback_injection_mutex);
        callback = _p->substitution_callback;
      }
      if(callback)
      {
        auto override_status = callback(*this);
        if(isStatusCompleted(override_status))
        {
          // don't execute the actual tick()
          substituted = true;
          new_status = override_status;
        }
      }
    }

    // Call the ACTUAL tick
    if(!substituted){
      new_status = tick();
    }
  }

  checkPostConditions(new_status);

  // injected post callback
  if(isStatusCompleted(new_status))
  {
    PostTickCallback  callback;
    {
      std::unique_lock lk(_p->callback_injection_mutex);
      callback = _p->post_condition_callback;
    }
    if(callback)
    {
      auto override_status = callback(*this, new_status);
      if(isStatusCompleted(override_status))
      {
        new_status = override_status;
      }
    }
  }

  setStatus(new_status);
  return new_status;
}

void TreeNode::haltNode()
{
  halt();

  const auto& parse_executor = _p->post_parsed[size_t(PostCond::ON_HALTED)];
  if (parse_executor)
  {
    Ast::Environment env = {config().blackboard, config().enums};
    parse_executor(env);
  }
}

void TreeNode::setStatus(NodeStatus new_status)
{
  if (new_status == NodeStatus::IDLE)
  {
    throw RuntimeError("Node [", name(),
                       "]: you are not allowed to set manually the status to IDLE. "
                       "If you know what you are doing (?) use resetStatus() instead.");
  }

  NodeStatus prev_status;
  {
    std::unique_lock<std::mutex> UniqueLock(_p->state_mutex);
    prev_status = _p->status;
    _p->status = new_status;
  }
  if (prev_status != new_status)
  {
    _p->state_condition_variable.notify_all();
    _p->state_change_signal.notify(std::chrono::high_resolution_clock::now(), *this,
                                   prev_status, new_status);
  }
}

TreeNode::PreScripts &TreeNode::preConditionsScripts() {
  return _p->pre_parsed;
}

TreeNode::PostScripts &TreeNode::postConditionsScripts() {
  return _p->post_parsed;
}

Expected<NodeStatus> TreeNode::checkPreConditions()
{
  // check the pre-conditions
  for (size_t index = 0; index < size_t(PreCond::COUNT_); index++)
  {
    PreCond preID = PreCond(index);
    const auto& parse_executor = _p->pre_parsed[index];
    if (!parse_executor)
    {
      continue;
    }
    Ast::Environment env = {config().blackboard, config().enums};
    auto result = parse_executor(env);
    // what to do if the condition is true
    if (result.cast<bool>())
    {
      // Some preconditions are applied only when the node is started
      if (!isStatusCompleted(_p->status))
      {
        if (preID == PreCond::FAILURE_IF)
        {
          return NodeStatus::FAILURE;
        }
        else if (preID == PreCond::SUCCESS_IF)
        {
          return NodeStatus::SUCCESS;
        }
        else if (preID == PreCond::SKIP_IF)
        {
          return NodeStatus::SKIPPED;
        }
      }
    }
    else   // condition is false
    {
      if (preID == PreCond::WHILE_TRUE)
      {
        if (_p->status == NodeStatus::RUNNING)
        {
          halt();
        }
        return NodeStatus::SKIPPED;
      }
    }
  }
  return nonstd::make_unexpected("");   // no precondition
}

void TreeNode::checkPostConditions(NodeStatus status)
{
  auto ExecuteScript = [this](const PostCond& cond) {
    const auto& parse_executor = _p->post_parsed[size_t(cond)];
    if (parse_executor)
    {
      Ast::Environment env = {config().blackboard, config().enums};
      parse_executor(env);
    }
  };

  if (status == NodeStatus::SUCCESS)
  {
    ExecuteScript(PostCond::ON_SUCCESS);
  }
  else if (status == NodeStatus::FAILURE)
  {
    ExecuteScript(PostCond::ON_FAILURE);
  }
  ExecuteScript(PostCond::ALWAYS);
}

void TreeNode::resetStatus()
{
  NodeStatus prev_status;
  {
    std::unique_lock<std::mutex> lock(_p->state_mutex);
    prev_status = _p->status;
    _p->status = NodeStatus::IDLE;
  }

  if (prev_status != NodeStatus::IDLE)
  {
    _p->state_condition_variable.notify_all();
    _p->state_change_signal.notify(std::chrono::high_resolution_clock::now(), *this,
                                   prev_status, NodeStatus::IDLE);
  }
}

NodeStatus TreeNode::status() const
{
  std::lock_guard<std::mutex> lock(_p->state_mutex);
  return _p->status;
}

NodeStatus TreeNode::waitValidStatus()
{
  std::unique_lock<std::mutex> lock(_p->state_mutex);

  while (isHalted())
  {
    _p->state_condition_variable.wait(lock);
  }
  return _p->status;
}

const std::string& TreeNode::name() const
{
  return _p->name;
}

bool TreeNode::isHalted() const
{
  return _p->status == NodeStatus::IDLE;
}

TreeNode::StatusChangeSubscriber
TreeNode::subscribeToStatusChange(TreeNode::StatusChangeCallback callback)
{
  return _p->state_change_signal.subscribe(std::move(callback));
}

void TreeNode::setPreTickFunction(PreTickCallback callback)
{
  std::unique_lock lk(_p->callback_injection_mutex);
  _p->substitution_callback = callback;
}

void TreeNode::setPostTickFunction(PostTickCallback callback)
{
  std::unique_lock lk(_p->callback_injection_mutex);
  _p->post_condition_callback = callback;
}

uint16_t TreeNode::UID() const
{
  return _p->config.uid;
}

const std::string &TreeNode::fullPath() const
{
  return _p->config.path;
}

const std::string& TreeNode::registrationName() const
{
  return _p->registration_ID;
}

const NodeConfig &TreeNode::config() const
{
  return _p->config;
}

NodeConfig &TreeNode::config()
{
  return _p->config;
}

StringView TreeNode::getRawPortValue(const std::string& key) const
{
  auto remap_it = _p->config.input_ports.find(key);
  if (remap_it == _p->config.input_ports.end())
  {
    remap_it = _p->config.output_ports.find(key);
    if (remap_it == _p->config.output_ports.end())
    {
      throw std::logic_error(StrCat("[", key, "] not found"));
    }
  }
  return remap_it->second;
}

bool TreeNode::isBlackboardPointer(StringView str)
{
  const auto size = str.size();
  if (size >= 3 && str.back() == '}')
  {
    if (str[0] == '{')
    {
      return true;
    }
    if (size >= 4 && str[0] == '$' && str[1] == '{')
    {
      return true;
    }
  }
  return false;
}

StringView TreeNode::stripBlackboardPointer(StringView str)
{
  const auto size = str.size();
  if (size >= 3 && str.back() == '}')
  {
    if (str[0] == '{')
    {
      return str.substr(1, size - 2);
    }
    if (str[0] == '$' && str[1] == '{')
    {
      return str.substr(2, size - 3);
    }
  }
  return {};
}

Expected<StringView> TreeNode::getRemappedKey(StringView port_name,
                                              StringView remapped_port)
{
  if (remapped_port == "=")
  {
    return {port_name};
  }
  if (isBlackboardPointer(remapped_port))
  {
    return {stripBlackboardPointer(remapped_port)};
  }
  return nonstd::make_unexpected("Not a blackboard pointer");
}

void TreeNode::emitWakeUpSignal()
{
  if (_p->wake_up)
  {
    _p->wake_up->emitSignal();
  }
}

bool TreeNode::requiresWakeUp() const
{
  return bool(_p->wake_up);
}

void TreeNode::setRegistrationID(StringView ID)
{
  _p->registration_ID.assign(ID.data(), ID.size());
}

void TreeNode::setWakeUpInstance(std::shared_ptr<WakeUpSignal> instance)
{
  _p->wake_up = instance;
}

void TreeNode::modifyPortsRemapping(const PortsRemapping& new_remapping)
{
  for (const auto& new_it : new_remapping)
  {
    auto it = _p->config.input_ports.find(new_it.first);
    if (it != _p->config.input_ports.end())
    {
      it->second = new_it.second;
    }
    it = _p->config.output_ports.find(new_it.first);
    if (it != _p->config.output_ports.end())
    {
      it->second = new_it.second;
    }
  }
}

template <>
std::string toStr<PreCond>(const PreCond& pre)
{
  switch (pre)
  {
    case PreCond::SUCCESS_IF:
      return "_successIf";
    case PreCond::FAILURE_IF:
      return "_failureIf";
    case PreCond::SKIP_IF:
      return "_skipIf";
    case PreCond::WHILE_TRUE:
      return "_while";
    default:
      return "Undefined";
  }
}

template <>
std::string toStr<PostCond>(const PostCond& pre)
{
  switch (pre)
  {
    case PostCond::ON_SUCCESS:
      return "_onSuccess";
    case PostCond::ON_FAILURE:
      return "_onFailure";
    case PostCond::ALWAYS:
      return "_post";
    case PostCond::ON_HALTED:
      return "_onHalted";
    default:
      return "Undefined";
  }
}

AnyPtrLocked BT::TreeNode::getLockedPortContent(const std::string &key)
{
  if(auto remapped_key = getRemappedKey(key, getRawPortValue(key)))
  {
    return _p->config.blackboard->getAnyLocked(std::string(*remapped_key));
  }
  return {};
}

}   // namespace BT
