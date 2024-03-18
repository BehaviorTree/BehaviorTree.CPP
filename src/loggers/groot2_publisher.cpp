#include "behaviortree_cpp/loggers/groot2_publisher.h"
#include "behaviortree_cpp/loggers/groot2_protocol.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "cppzmq/zmq.hpp"
#include "cppzmq/zmq_addon.hpp"

namespace BT
{
//------------------------------------------------------
std::mutex Groot2Publisher::used_ports_mutex;
std::set<unsigned> Groot2Publisher::used_ports;

enum
{
  IDLE_FROM_SUCCESS = 10 + static_cast<int>(NodeStatus::SUCCESS),
  IDLE_FROM_FAILURE = 10 + static_cast<int>(NodeStatus::FAILURE),
  IDLE_FROM_RUNNING = 10 + static_cast<int>(NodeStatus::RUNNING)
};

struct Transition
{
  // when serializing, we will remove the initial time and serialize only
  // 6 bytes, instead of 8
  uint64_t timestamp_usec;
  // if you have more than 64.000 nodes, you are doing something wrong :)
  uint16_t node_uid;
  // enough bits to contain NodeStatus
  uint8_t status;

  uint8_t padding[5];
};

std::array<char, 16> CreateRandomUUID()
{
  std::mt19937 gen;
  std::uniform_int_distribution<uint32_t> dist;
  std::array<char, 16> out;
  char* bytes = out.data();
  for(int i = 0; i < 16; i += 4)
  {
    *reinterpret_cast<uint32_t*>(bytes + i) = dist(gen);
  }
  // variant must be 10xxxxxx
  bytes[8] &= 0xBF;
  bytes[8] |= 0x80;

  // version must be 0100xxxx
  bytes[6] &= 0x4F;
  bytes[6] |= 0x40;

  return out;
}

struct Groot2Publisher::PImpl
{
  PImpl() : context(), server(context, ZMQ_REP), publisher(context, ZMQ_PUB)
  {
    server.set(zmq::sockopt::linger, 0);
    publisher.set(zmq::sockopt::linger, 0);

    int timeout_rcv = 100;
    server.set(zmq::sockopt::rcvtimeo, timeout_rcv);
    publisher.set(zmq::sockopt::rcvtimeo, timeout_rcv);

    int timeout_ms = 1000;
    server.set(zmq::sockopt::sndtimeo, timeout_ms);
    publisher.set(zmq::sockopt::sndtimeo, timeout_ms);
  }

  unsigned server_port = 0;
  std::string server_address;
  std::string publisher_address;

  std::string tree_xml;

  std::atomic_bool active_server = false;
  std::thread server_thread;

  std::mutex status_mutex;

  std::string status_buffer;
  // each element of this map points to a character in _p->status_buffer
  std::unordered_map<uint16_t, char*> status_buffermap;

  // weak reference to the tree.
  std::unordered_map<std::string, std::weak_ptr<BT::Tree::Subtree>> subtrees;
  std::unordered_map<uint16_t, std::weak_ptr<BT::TreeNode>> nodes_by_uid;

  std::mutex hooks_map_mutex;
  std::unordered_map<uint16_t, Monitor::Hook::Ptr> pre_hooks;
  std::unordered_map<uint16_t, Monitor::Hook::Ptr> post_hooks;

  std::chrono::system_clock::time_point last_heartbeat;
  std::chrono::milliseconds max_heartbeat_delay = std::chrono::milliseconds(5000);

  std::atomic_bool recording = false;
  std::deque<Transition> transitions_buffer;
  std::chrono::microseconds recording_fist_time;

  std::thread heartbeat_thread;

  zmq::context_t context;
  zmq::socket_t server;
  zmq::socket_t publisher;
};

Groot2Publisher::Groot2Publisher(const BT::Tree& tree, unsigned server_port)
  : StatusChangeLogger(tree.rootNode()), _p(new PImpl())
{
  _p->server_port = server_port;

  {
    std::unique_lock<std::mutex> lk(Groot2Publisher::used_ports_mutex);
    if(Groot2Publisher::used_ports.count(server_port) != 0 ||
       Groot2Publisher::used_ports.count(server_port + 1) != 0)
    {
      auto msg = StrCat("Another instance of Groot2Publisher is using port ",
                        std::to_string(server_port));
      throw LogicError(msg);
    }
    Groot2Publisher::used_ports.insert(server_port);
    Groot2Publisher::used_ports.insert(server_port + 1);
  }

  _p->tree_xml = WriteTreeToXML(tree, true, true);

  //-------------------------------
  // Prepare the status buffer
  size_t node_count = 0;
  for(const auto& subtree : tree.subtrees)
  {
    node_count += subtree->nodes.size();
  }
  _p->status_buffer.resize(3 * node_count);

  unsigned ptr_offset = 0;
  char* buffer_ptr = _p->status_buffer.data();

  for(const auto& subtree : tree.subtrees)
  {
    auto name =
        subtree->instance_name.empty() ? subtree->tree_ID : subtree->instance_name;
    _p->subtrees.insert({ name, subtree });

    for(const auto& node : subtree->nodes)
    {
      _p->nodes_by_uid.insert({ node->UID(), node });

      ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset, node->UID());
      _p->status_buffermap.insert({ node->UID(), buffer_ptr + ptr_offset });
      ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset, uint8_t(NodeStatus::IDLE));
    }
  }
  //-------------------------------
  _p->server_address = StrCat("tcp://*:", std::to_string(server_port));
  _p->publisher_address = StrCat("tcp://*:", std::to_string(server_port + 1));

  _p->server.bind(_p->server_address.c_str());
  _p->publisher.bind(_p->publisher_address.c_str());

  _p->server_thread = std::thread(&Groot2Publisher::serverLoop, this);
  _p->heartbeat_thread = std::thread(&Groot2Publisher::heartbeatLoop, this);
}

void Groot2Publisher::setMaxHeartbeatDelay(std::chrono::milliseconds delay)
{
  _p->max_heartbeat_delay = delay;
}

std::chrono::milliseconds Groot2Publisher::maxHeartbeatDelay() const
{
  return _p->max_heartbeat_delay;
}

Groot2Publisher::~Groot2Publisher()
{
  removeAllHooks();

  _p->active_server = false;
  if(_p->server_thread.joinable())
  {
    _p->server_thread.join();
  }

  if(_p->heartbeat_thread.joinable())
  {
    _p->heartbeat_thread.join();
  }

  flush();

  {
    std::unique_lock<std::mutex> lk(Groot2Publisher::used_ports_mutex);
    Groot2Publisher::used_ports.erase(_p->server_port);
    Groot2Publisher::used_ports.erase(_p->server_port + 1);
  }
}

void Groot2Publisher::callback(Duration ts, const TreeNode& node, NodeStatus prev_status,
                               NodeStatus new_status)
{
  std::unique_lock<std::mutex> lk(_p->status_mutex);
  auto status = static_cast<char>(new_status);

  if(new_status == NodeStatus::IDLE)
  {
    status = 10 + static_cast<char>(prev_status);
  }
  *(_p->status_buffermap.at(node.UID())) = status;

  if(_p->recording)
  {
    Transition trans;
    trans.node_uid = node.UID();
    trans.status = static_cast<uint8_t>(new_status);
    auto timestamp = ts - _p->recording_fist_time;
    trans.timestamp_usec =
        std::chrono::duration_cast<std::chrono::microseconds>(timestamp).count();
    _p->transitions_buffer.push_back(trans);
    while(_p->transitions_buffer.size() > 1000)
    {
      _p->transitions_buffer.pop_front();
    }
  }
}

void Groot2Publisher::flush()
{
  // nothing to do here...
}

void Groot2Publisher::serverLoop()
{
  auto const serialized_uuid = CreateRandomUUID();

  _p->active_server = true;
  auto& socket = _p->server;

  auto sendErrorReply = [&socket](const std::string& msg) {
    zmq::multipart_t error_msg;
    error_msg.addstr("error");
    error_msg.addstr(msg);
    error_msg.send(socket);
  };

  // initialize _p->last_heartbeat
  _p->last_heartbeat = std::chrono::system_clock::now();

  while(_p->active_server)
  {
    zmq::multipart_t requestMsg;
    if(!requestMsg.recv(socket) || requestMsg.size() == 0)
    {
      continue;
    }
    // this heartbeat will help establishing if Groot is connected or not
    _p->last_heartbeat = std::chrono::system_clock::now();

    std::string const request_str = requestMsg[0].to_string();
    if(request_str.size() != Monitor::RequestHeader::size())
    {
      sendErrorReply("wrong request header");
      continue;
    }

    auto request_header = Monitor::DeserializeRequestHeader(request_str);

    Monitor::ReplyHeader reply_header;
    reply_header.request = request_header;
    reply_header.request.protocol = Monitor::kProtocolID;
    reply_header.tree_id = serialized_uuid;

    zmq::multipart_t reply_msg;
    reply_msg.addstr(Monitor::SerializeHeader(reply_header));

    switch(request_header.type)
    {
      case Monitor::RequestType::FULLTREE: {
        reply_msg.addstr(_p->tree_xml);
      }
      break;

      case Monitor::RequestType::STATUS: {
        std::unique_lock<std::mutex> lk(_p->status_mutex);
        reply_msg.addstr(_p->status_buffer);
      }
      break;

      case Monitor::RequestType::BLACKBOARD: {
        if(requestMsg.size() != 2)
        {
          sendErrorReply("must be 2 parts message");
          continue;
        }
        std::string const bb_names_str = requestMsg[1].to_string();
        auto msg = generateBlackboardsDump(bb_names_str);
        reply_msg.addmem(msg.data(), msg.size());
      }
      break;

      case Monitor::RequestType::HOOK_INSERT: {
        if(requestMsg.size() != 2)
        {
          sendErrorReply("must be 2 parts message");
          continue;
        }

        auto InsertHook = [this](nlohmann::json const& json) {
          uint16_t const node_uid = json["uid"].get<uint16_t>();
          Position const pos = static_cast<Position>(json["position"].get<int>());

          if(auto hook = getHook(pos, node_uid))
          {
            std::unique_lock<std::mutex> lk(hook->mutex);
            bool was_interactive = (hook->mode == Monitor::Hook::Mode::BREAKPOINT);
            BT::Monitor::from_json(json, *hook);

            // if it WAS interactive and it is not anymore, unlock it
            if(was_interactive && (hook->mode == Monitor::Hook::Mode::REPLACE))
            {
              hook->ready = true;
              lk.unlock();
              hook->wakeup.notify_all();
            }
          }
          else  // if not found, create a new one
          {
            auto new_hook = std::make_shared<Monitor::Hook>();
            BT::Monitor::from_json(json, *new_hook);
            insertHook(new_hook);
          }
        };

        auto const received_json = nlohmann::json::parse(requestMsg[1].to_string());

        // the json may contain a Hook or an array of Hooks
        if(received_json.is_array())
        {
          for(auto const& json : received_json)
          {
            InsertHook(json);
          }
        }
        else
        {
          InsertHook(received_json);
        }
      }
      break;

      case Monitor::RequestType::BREAKPOINT_UNLOCK: {
        if(requestMsg.size() != 2)
        {
          sendErrorReply("must be 2 parts message");
          continue;
        }

        auto json = nlohmann::json::parse(requestMsg[1].to_string());
        uint16_t node_uid = json.at("uid").get<uint16_t>();
        std::string status_str = json.at("desired_status").get<std::string>();
        auto position = static_cast<Position>(json.at("position").get<int>());
        bool remove = json.at("remove_when_done").get<bool>();

        NodeStatus desired_status = NodeStatus::SKIPPED;
        if(status_str == "SUCCESS")
        {
          desired_status = NodeStatus::SUCCESS;
        }
        else if(status_str == "FAILURE")
        {
          desired_status = NodeStatus::FAILURE;
        }

        if(!unlockBreakpoint(position, uint16_t(node_uid), desired_status, remove))
        {
          sendErrorReply("Node ID not found");
          continue;
        }
      }
      break;

      case Monitor::RequestType::REMOVE_ALL_HOOKS: {
        removeAllHooks();
      }
      break;

      case Monitor::RequestType::DISABLE_ALL_HOOKS: {
        enableAllHooks(false);
      }
      break;

      case Monitor::RequestType::HOOK_REMOVE: {
        if(requestMsg.size() != 2)
        {
          sendErrorReply("must be 2 parts message");
          continue;
        }

        auto json = nlohmann::json::parse(requestMsg[1].to_string());
        uint16_t node_uid = json.at("uid").get<uint16_t>();
        auto position = static_cast<Position>(json.at("position").get<int>());

        if(!removeHook(position, uint16_t(node_uid)))
        {
          sendErrorReply("Node ID not found");
          continue;
        }
      }
      break;

      case Monitor::RequestType::HOOKS_DUMP: {
        std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
        auto json_out = nlohmann::json::array();
        for(auto [node_uid, breakpoint] : _p->pre_hooks)
        {
          json_out.push_back(*breakpoint);
        }
        reply_msg.addstr(json_out.dump());
      }
      break;

      case Monitor::RequestType::TOGGLE_RECORDING: {
        if(requestMsg.size() != 2)
        {
          sendErrorReply("must be 2 parts message");
          continue;
        }

        auto const cmd = (requestMsg[1].to_string());
        if(cmd == "start")
        {
          _p->recording = true;
          // to keep the first time for callback
          _p->recording_fist_time = std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch());
          // to send consistent time for client
          auto now = std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::system_clock::now().time_since_epoch());
          reply_msg.addstr(std::to_string(now.count()));
          std::unique_lock<std::mutex> lk(_p->status_mutex);
          _p->transitions_buffer.clear();
        }
        else if(cmd == "stop")
        {
          _p->recording = false;
        }
      }
      break;

      case Monitor::RequestType::GET_TRANSITIONS: {
        thread_local std::string trans_buffer;
        trans_buffer.resize(9 * _p->transitions_buffer.size());

        std::unique_lock<std::mutex> lk(_p->status_mutex);
        size_t offset = 0;
        for(const auto& trans : _p->transitions_buffer)
        {
          std::memcpy(&trans_buffer[offset], &trans.timestamp_usec, 6);
          offset += 6;
          std::memcpy(&trans_buffer[offset], &trans.node_uid, 2);
          offset += 2;
          std::memcpy(&trans_buffer[offset], &trans.status, 1);
          offset += 1;
        }
        _p->transitions_buffer.clear();
        trans_buffer.resize(offset);
        reply_msg.addstr(trans_buffer);
      }
      break;

      default: {
        sendErrorReply("Request not recognized");
        continue;
      }
    }
    // send the reply
    reply_msg.send(socket);
  }
}

void BT::Groot2Publisher::enableAllHooks(bool enable)
{
  std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
  for(auto& [node_uid, hook] : _p->pre_hooks)
  {
    std::unique_lock<std::mutex> lk(hook->mutex);
    hook->enabled = enable;
    // when disabling, remember to wake up blocked ones
    if(!hook->enabled && hook->mode == Monitor::Hook::Mode::BREAKPOINT)
    {
      lk.unlock();
      hook->wakeup.notify_all();
    }
  }
}

void Groot2Publisher::heartbeatLoop()
{
  bool has_heartbeat = true;

  while(_p->active_server)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto now = std::chrono::system_clock::now();
    bool prev_heartbeat = has_heartbeat;

    has_heartbeat = (now - _p->last_heartbeat < _p->max_heartbeat_delay);

    // if we loose or gain heartbeat, disable/enable all breakpoints
    if(has_heartbeat != prev_heartbeat)
    {
      enableAllHooks(has_heartbeat);
    }
  }
}

std::vector<uint8_t> Groot2Publisher::generateBlackboardsDump(const std::string& bb_list)
{
  auto json = nlohmann::json();
  auto const bb_names = BT::splitString(bb_list, ';');
  for(auto name : bb_names)
  {
    std::string const bb_name(name);
    auto it = _p->subtrees.find(bb_name);

    if(it != _p->subtrees.end())
    {
      // lock the weak pointer
      if(auto subtree = it->second.lock())
      {
        json[bb_name] = ExportBlackboardToJSON(*subtree->blackboard);
      }
    }
  }
  return nlohmann::json::to_msgpack(json);
}

bool Groot2Publisher::insertHook(std::shared_ptr<Monitor::Hook> hook)
{
  auto const node_uid = hook->node_uid;
  auto it = _p->nodes_by_uid.find(node_uid);
  if(it == _p->nodes_by_uid.end())
  {
    return false;
  }
  TreeNode::Ptr node = it->second.lock();
  if(!node)
  {
    return false;
  }

  auto injectedCallback = [hook, this](TreeNode& node) -> NodeStatus {
    std::unique_lock<std::mutex> lk(hook->mutex);
    if(!hook->enabled)
    {
      return NodeStatus::SKIPPED;
    }

    // Notify that a breakpoint was reached, using the _p->publisher
    Monitor::RequestHeader breakpoint_request(Monitor::BREAKPOINT_REACHED);
    zmq::multipart_t request_msg;
    request_msg.addstr(Monitor::SerializeHeader(breakpoint_request));
    request_msg.addstr(std::to_string(hook->node_uid));
    request_msg.send(_p->publisher);

    // wait until someone wake us up
    if(hook->mode == Monitor::Hook::Mode::BREAKPOINT)
    {
      hook->wakeup.wait(lk, [hook]() { return hook->ready || !hook->enabled; });

      hook->ready = false;
      // wait was unblocked but it could be the breakpoint becoming disabled.
      // in this case, just skip
      if(!hook->enabled)
      {
        return NodeStatus::SKIPPED;
      }
    }

    if(hook->remove_when_done)
    {
      // self-destruction at the end of this lambda function
      std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
      _p->pre_hooks.erase(hook->node_uid);
      node.setPreTickFunction({});
    }
    return hook->desired_status;
  };

  std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
  _p->pre_hooks[node_uid] = hook;
  node->setPreTickFunction(injectedCallback);

  return true;
}

bool Groot2Publisher::unlockBreakpoint(Position pos, uint16_t node_uid, NodeStatus result,
                                       bool remove)
{
  auto it = _p->nodes_by_uid.find(node_uid);
  if(it == _p->nodes_by_uid.end())
  {
    return false;
  }
  TreeNode::Ptr node = it->second.lock();
  if(!node)
  {
    return false;
  }

  auto hook = getHook(pos, node_uid);
  if(!hook)
  {
    return false;
  }

  {
    std::unique_lock<std::mutex> lk(hook->mutex);
    hook->desired_status = result;
    hook->remove_when_done |= remove;
    if(hook->mode == Monitor::Hook::Mode::BREAKPOINT)
    {
      hook->ready = true;
      lk.unlock();
      hook->wakeup.notify_all();
    }
  }
  return true;
}

bool Groot2Publisher::removeHook(Position pos, uint16_t node_uid)
{
  auto it = _p->nodes_by_uid.find(node_uid);
  if(it == _p->nodes_by_uid.end())
  {
    return false;
  }
  TreeNode::Ptr node = it->second.lock();
  if(!node)
  {
    return false;
  }

  auto hook = getHook(pos, node_uid);
  if(!hook)
  {
    return false;
  }

  {
    std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
    _p->pre_hooks.erase(node_uid);
  }
  node->setPreTickFunction({});

  // Disable breakpoint, if it was interactive and blocked
  {
    std::unique_lock<std::mutex> lk(hook->mutex);
    if(hook->mode == Monitor::Hook::Mode::BREAKPOINT)
    {
      hook->enabled = false;
      lk.unlock();
      hook->wakeup.notify_all();
    }
  }
  return true;
}

void Groot2Publisher::removeAllHooks()
{
  std::vector<uint16_t> uids;

  for(auto pos : { Position::PRE, Position::POST })
  {
    uids.clear();
    auto hooks = pos == Position::PRE ? &_p->pre_hooks : &_p->post_hooks;
    std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
    if(!hooks->empty())
    {
      uids.reserve(hooks->size());
      for(auto [node_uid, _] : *hooks)
      {
        uids.push_back(node_uid);
      }

      lk.unlock();
      for(auto node_uid : uids)
      {
        removeHook(pos, node_uid);
      }
    }
  }
}

Monitor::Hook::Ptr Groot2Publisher::getHook(Position pos, uint16_t node_uid)
{
  auto hooks = pos == Position::PRE ? &_p->pre_hooks : &_p->post_hooks;
  std::unique_lock<std::mutex> lk(_p->hooks_map_mutex);
  auto bk_it = hooks->find(node_uid);
  if(bk_it == hooks->end())
  {
    return {};
  }
  return bk_it->second;
}

}  // namespace BT
