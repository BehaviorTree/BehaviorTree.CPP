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

TreeNode::TreeNode(std::string name, NodeConfig config) :
  name_(std::move(name)),
  status_(NodeStatus::IDLE),
  config_(std::move(config))
{}

NodeStatus TreeNode::executeTick()
{
  auto new_status = status_;

  // injected pre-callback
  if(status_ == NodeStatus::IDLE)
  {
    PreTickCallback  callback;
    {
      std::unique_lock lk(callback_injection_mutex_);
      callback = pre_condition_callback_;
    }
    if(callback)
    {
      auto override_status = callback(*this);
      if(isStatusCompleted(override_status))
      {
        // return immediately and don't execute the actual tick()
        new_status = override_status;
        setStatus(new_status);
        return new_status;
      }
    }
  }

  // a pre-condition may return the new status.
  // In this case it override the actual tick()
  if (auto precond = checkPreConditions())
  {
    new_status = precond.value();
  }
  else
  {
    //Call the actual tick
    new_status = tick();
  }

  checkPostConditions(new_status);

  // injected post callback
  if(isStatusCompleted(new_status))
  {
    PostTickCallback  callback;
    {
      std::unique_lock lk(callback_injection_mutex_);
      callback = post_condition_callback_;
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

  const auto& parse_executor = post_parsed_[size_t(PostCond::ON_HALTED)];
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
    std::unique_lock<std::mutex> UniqueLock(state_mutex_);
    prev_status = status_;
    status_ = new_status;
  }
  if (prev_status != new_status)
  {
    state_condition_variable_.notify_all();
    state_change_signal_.notify(std::chrono::high_resolution_clock::now(), *this,
                                prev_status, new_status);
  }
}

Expected<NodeStatus> TreeNode::checkPreConditions()
{
  // check the pre-conditions
  for (size_t index = 0; index < size_t(PreCond::COUNT_); index++)
  {
    PreCond preID = PreCond(index);
    const auto& parse_executor = pre_parsed_[index];
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
      if (status_ == NodeStatus::IDLE)
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
        if (status_ == NodeStatus::RUNNING)
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
    const auto& parse_executor = post_parsed_[size_t(cond)];
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
    std::unique_lock<std::mutex> lock(state_mutex_);
    prev_status = status_;
    status_ = NodeStatus::IDLE;
  }

  if (prev_status != NodeStatus::IDLE)
  {
    state_condition_variable_.notify_all();
    state_change_signal_.notify(std::chrono::high_resolution_clock::now(), *this,
                                prev_status, NodeStatus::IDLE);
  }
}

NodeStatus TreeNode::status() const
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  return status_;
}

NodeStatus TreeNode::waitValidStatus()
{
  std::unique_lock<std::mutex> lock(state_mutex_);

  while (isHalted())
  {
    state_condition_variable_.wait(lock);
  }
  return status_;
}

const std::string& TreeNode::name() const
{
  return name_;
}

bool TreeNode::isHalted() const
{
  return status_ == NodeStatus::IDLE;
}

TreeNode::StatusChangeSubscriber
TreeNode::subscribeToStatusChange(TreeNode::StatusChangeCallback callback)
{
  return state_change_signal_.subscribe(std::move(callback));
}

void TreeNode::setPreTickFunction(PreTickCallback callback)
{
  std::unique_lock lk(callback_injection_mutex_);
  pre_condition_callback_ = callback;
}

void TreeNode::setPostTickFunction(PostTickCallback callback)
{
  std::unique_lock lk(callback_injection_mutex_);
  post_condition_callback_ = callback;
}

uint16_t TreeNode::UID() const
{
  return config_.uid;
}

const std::string &TreeNode::fullPath() const
{
  return config_.path;
}

const std::string& TreeNode::registrationName() const
{
  return registration_ID_;
}

const NodeConfig& TreeNode::config() const
{
  return config_;
}

StringView TreeNode::getRawPortValue(const std::string& key) const
{
  auto remap_it = config_.input_ports.find(key);
  if (remap_it == config_.input_ports.end())
  {
    throw std::logic_error(StrCat("getInput() failed because "
                                  "NodeConfig::input_ports "
                                  "does not contain the key: [",
                                  key, "]"));
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
  if (wake_up_)
  {
    wake_up_->emitSignal();
  }
}

bool TreeNode::requiresWakeUp() const
{
  return bool(wake_up_);
}

void TreeNode::setRegistrationID(StringView ID)
{
  registration_ID_.assign(ID.data(), ID.size());
}

void TreeNode::setWakeUpInstance(std::shared_ptr<WakeUpSignal> instance)
{
  wake_up_ = instance;
}

void TreeNode::modifyPortsRemapping(const PortsRemapping& new_remapping)
{
  for (const auto& new_it : new_remapping)
  {
    auto it = config_.input_ports.find(new_it.first);
    if (it != config_.input_ports.end())
    {
      it->second = new_it.second;
    }
    it = config_.output_ports.find(new_it.first);
    if (it != config_.output_ports.end())
    {
      it->second = new_it.second;
    }
  }
}

template <>
std::string toStr<PreCond>(PreCond pre)
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
std::string toStr<PostCond>(PostCond pre)
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

}   // namespace BT
