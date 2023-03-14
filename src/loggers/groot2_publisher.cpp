#include <future>
#include "behaviortree_cpp/json_export.h"
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

std::array<char,16> CreateRandomUUID()
{
  std::mt19937 gen;
  std::uniform_int_distribution<uint32_t> dist;
  std::array<char,16> out;
  char* bytes = out.data();
  for (int i = 0; i < 16; i += 4)
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

struct Groot2Publisher::Pimpl
{
  Pimpl() : context(), server(context, ZMQ_REP), publisher(context, ZMQ_PUB)
  {
    server.set(zmq::sockopt::linger, 0);
    publisher.set(zmq::sockopt::linger, 0);

    int timeout_rcv = 100;
    server.set(zmq::sockopt::rcvtimeo, timeout_rcv);
    publisher.set(zmq::sockopt::rcvtimeo, timeout_rcv);

    int timeout_ms = 1000;
    server.set(zmq::sockopt::sndtimeo, timeout_ms);
    publisher.set(zmq::sockopt::rcvtimeo, timeout_rcv);
  }

  zmq::context_t context;
  zmq::socket_t server;
  zmq::socket_t publisher;
};

Groot2Publisher::Groot2Publisher(const BT::Tree& tree,
                                 unsigned server_port) :
  StatusChangeLogger(tree.rootNode()),
  server_port_(server_port),
  zmq_(new Pimpl())
{
  {
    std::unique_lock<std::mutex> lk(Groot2Publisher::used_ports_mutex);
    if(Groot2Publisher::used_ports.count(server_port) != 0 ||
        Groot2Publisher::used_ports.count(server_port+1 != 0))
    {
      auto msg = StrCat("Another instance of Groot2Publisher is using port ",
                        std::to_string(server_port));
      throw LogicError(msg);
    }
    Groot2Publisher::used_ports.insert(server_port);
    Groot2Publisher::used_ports.insert(server_port+1);
  }

  tree_xml_ = WriteTreeToXML(tree, true);

  //-------------------------------
  // Prepare the status buffer
  size_t node_count = 0;
  for(const auto& subtree: tree.subtrees)
  {
    node_count += subtree->nodes.size();
  }
  status_buffer_.resize(3 * node_count);

  unsigned ptr_offset = 0;
  char* buffer_ptr = status_buffer_.data();

  for(const auto& subtree: tree.subtrees)
  {
    subtrees_.insert( {subtree->instance_name, subtree} );

    for(const auto& node: subtree->nodes)
    {
      nodes_by_uid_.insert( {node->UID(), node} );

      ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset,
                                       node->UID());
      buffer_ptr_.insert( {node->UID(), buffer_ptr + ptr_offset} );
      ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset,
                                       uint8_t(NodeStatus::IDLE));
    }
  }
  //-------------------------------
  server_address_ = StrCat("tcp://*:", std::to_string(server_port));
  publisher_address_ = StrCat("tcp://*:", std::to_string(server_port+1));

  zmq_->server.bind(server_address_.c_str());
  zmq_->publisher.bind(publisher_address_.c_str());

  server_thread_ = std::thread(&Groot2Publisher::serverLoop, this);
  heartbeat_thread_ = std::thread(&Groot2Publisher::heartbeatLoop, this);
}

Groot2Publisher::~Groot2Publisher()
{
  removeAllBreakpoints();

  active_server_ = false;
  if (server_thread_.joinable())
  {
    server_thread_.join();
  }

  if (heartbeat_thread_.joinable())
  {
    heartbeat_thread_.join();
  }

  flush();
  delete zmq_;

  {
    std::unique_lock<std::mutex> lk(Groot2Publisher::used_ports_mutex);
    Groot2Publisher::used_ports.erase(server_port_);
  }
}

void Groot2Publisher::callback(Duration, const TreeNode& node,
                               NodeStatus, NodeStatus status)
{
  std::unique_lock<std::mutex> lk(status_mutex_);
  *(buffer_ptr_.at(node.UID())) = static_cast<char>(status);
}

void Groot2Publisher::flush()
{
  // nothing to do here...
}

void Groot2Publisher::serverLoop()
{
  auto const serialized_uuid = CreateRandomUUID();

  active_server_ = true;
  auto& socket = zmq_->server;

  auto sendErrorReply = [&socket](const std::string& msg)
  {
    zmq::multipart_t error_msg;
    error_msg.addstr("error");
    error_msg.addstr(msg);
    error_msg.send(socket);
  };

  // initialize last_heartbeat_
  last_heartbeat_ = std::chrono::system_clock::now();

  while (active_server_)
  {
    zmq::multipart_t requestMsg;
    if( !requestMsg.recv(socket) || requestMsg.size() == 0)
    {
      continue;
    }
    // this heartbeat will help establishing if Groot is connected or not
    last_heartbeat_ = std::chrono::system_clock::now();

    std::string const request_str = requestMsg[0].to_string();
    if(request_str.size() != Monitor::RequestHeader::size())
    {
      sendErrorReply("wrong request header");
      continue;
    }

    auto request_header = Monitor::DeserializeRequestHeader(request_str);

    Monitor::ReplyHeader reply_header;
    reply_header.request = request_header;
    reply_header.tree_id = serialized_uuid;

    zmq::multipart_t reply_msg;
    reply_msg.addstr( Monitor::SerializeHeader(reply_header) );

    switch(request_header.type)
    {
      case Monitor::RequestType::FULLTREE:
      {
        reply_msg.addstr( tree_xml_ );
      } break;

      case Monitor::RequestType::STATUS:
      {
        std::unique_lock<std::mutex> lk(status_mutex_);
        reply_msg.addstr( status_buffer_ );
      } break;

      case Monitor::RequestType::BLACKBOARD:
      {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          continue;
        }
        std::string const bb_names_str = requestMsg[1].to_string();
        auto msg = generateBlackboardsDump(bb_names_str);
        reply_msg.addmem(msg.data(), msg.size());
      } break;

      case Monitor::RequestType::BREAKPOINT_INSERT:
      {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          continue;
        }

        auto const received_json = nlohmann::json::parse(requestMsg[1].to_string());
        uint16_t const node_uid = received_json["uid"].get<uint16_t>();

        // if it exists already, we want to modify this in place
        if(auto breakpoint = getBreakpoint(node_uid))
        {
          std::unique_lock<std::mutex> lk(breakpoint->mutex);
          bool was_interactive = breakpoint->is_interactive;
          BT::Monitor::from_json(received_json, *breakpoint);

          // if it WAS interactive and it is not anymore, unlock it
          if(was_interactive && !breakpoint->is_interactive)
          {
            breakpoint->ready = true;
            lk.unlock();
            breakpoint->wakeup.notify_all();
          }
        }
        else // if not found, create a new one
        {
          auto new_breakpoint = std::make_shared<Monitor::Breakpoint>();
          BT::Monitor::from_json(received_json, *new_breakpoint);

          if(!insertBreakpoint(new_breakpoint))
          {
            sendErrorReply("Node ID not found");
            continue;
          }
        }
      } break;

      case Monitor::RequestType::BREAKPOINT_UNLOCK:
      {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          continue;
        }
        auto str_parts = splitString(requestMsg[1].to_string_view(), ';');

        if(str_parts.size() < 2) {
          sendErrorReply("String must have at least 2 parts");
          continue;
        }

        int node_uid = std::stoi( std::string(str_parts[0]) );
        const auto status_str = str_parts[1];
        bool remove = str_parts.size() >= 3 && str_parts[2] == "remove";

        NodeStatus desired_status = NodeStatus::SKIPPED;
        if( status_str == "SUCCESS")
        {
          desired_status = NodeStatus::SUCCESS;
        }
        else if( status_str == "FAILURE")
        {
          desired_status = NodeStatus::FAILURE;
        }

        if(!unlockBreakpoint(uint16_t(node_uid), desired_status, remove))
        {
          sendErrorReply("Node ID not found");
          continue;
        }
      } break;

      case Monitor::RequestType::REMOVE_ALL_BREAKPOINTS:
      {
        removeAllBreakpoints();
      } break;

      case Monitor::RequestType::DISABLE_ALL_BREAKPOINTS:
      {
        enableAllBreakpoints(false);
      } break;

      case Monitor::RequestType::BREAKPOINT_REMOVE:
      {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          continue;
        }
        int node_uid = std::stoi( requestMsg[1].to_string() );
        if(!removeBreakpoint(uint16_t(node_uid)))
        {
          sendErrorReply("Node ID not found");
          continue;
        }
      } break;

      case Monitor::RequestType::BREAKPOINTS_DUMP:
      {
        std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
        auto json_breakpoints = nlohmann::json::array();
        for(auto [node_uid, breakpoint]: pre_breakpoints_)
        {
          json_breakpoints.push_back( *breakpoint );
        }
        reply_msg.addstr( json_breakpoints.dump() );
      } break;
      default: {
        sendErrorReply("Request not recognized");
        continue;
      }
    }
    // send the reply
    reply_msg.send(socket);
  }
}

void BT::Groot2Publisher::enableAllBreakpoints(bool enable)
{
  std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
  for(auto& [node_uid, breakpoint]: pre_breakpoints_)
  {
    std::unique_lock<std::mutex> lk(breakpoint->mutex);
    breakpoint->enabled = enable;
    // when disabling, remember to wake up blocked ones
    if(!breakpoint->enabled && breakpoint->is_interactive)
    {
      lk.unlock();
      breakpoint->wakeup.notify_all();
    }
  }
}

void Groot2Publisher::heartbeatLoop()
{
  bool has_heartbeat = true;

  while (active_server_)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto now = std::chrono::system_clock::now();
    bool prev_heartbeat = has_heartbeat;

    has_heartbeat = ( now - last_heartbeat_ < std::chrono::milliseconds(5000));

    // if we loose or gain heartbeat, disable/enable all breakpoints
    if(has_heartbeat != prev_heartbeat)
    {
      enableAllBreakpoints(has_heartbeat);
    }
  }
}

std::vector<uint8_t>
Groot2Publisher::generateBlackboardsDump(const std::string &bb_list)
{
  auto json = nlohmann::json();
  auto const bb_names = BT::splitString(bb_list, ';');
  for(auto name: bb_names)
  {
    std::string const bb_name(name);
    auto it = subtrees_.find(bb_name);

    if(it != subtrees_.end())
    {
      // lock the weak pointer
      if(auto subtree = it->second.lock()) {
        json[bb_name] = ExportBlackboardToJSON(*subtree->blackboard);
      }
    }
  }
  return nlohmann::json::to_msgpack(json);
}

bool Groot2Publisher::insertBreakpoint(std::shared_ptr<Monitor::Breakpoint> breakpoint)
{
  auto const node_uid = breakpoint->node_uid;
  auto it = nodes_by_uid_.find(node_uid);
  if( it == nodes_by_uid_.end())
  {
    return false;
  }
  TreeNode::Ptr node = it->second.lock();
  if(!node)
  {
    return false;
  }

  auto injectedCallback = [breakpoint, this](TreeNode& node) -> NodeStatus
  {
    std::unique_lock<std::mutex> lk(breakpoint->mutex);
    if(!breakpoint->enabled)
    {
      return NodeStatus::SKIPPED;
    }

    // Notify that a breakpoint was reached, using the zmq_->publisher
    Monitor::RequestHeader breakpoint_request(Monitor::BREAKPOINT_REACHED);
    zmq::multipart_t request_msg;
    request_msg.addstr( Monitor::SerializeHeader(breakpoint_request) );
    request_msg.addstr(std::to_string(breakpoint->node_uid));
    request_msg.send(zmq_->publisher);

    // wait until someone wake us up
    if(breakpoint->is_interactive)
    {
      breakpoint->wakeup.wait(lk, [breakpoint]() {
        return breakpoint->ready || !breakpoint->enabled;  } );

      breakpoint->ready = false;
      // wait was unblocked but it could be the breakpoint becoming disabled.
      // in this case, just skbreakpointip
      if(!breakpoint->enabled) {
        return NodeStatus::SKIPPED;
      }
    }

    if(breakpoint->remove_when_done)
    {
      // self-destruction at the end of this lambda function
      std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
      pre_breakpoints_.erase(breakpoint->node_uid);
      node.setPreTickFunction({});
    }
    return breakpoint->desired_status;
  };

  std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
  pre_breakpoints_[node_uid] = breakpoint;
  node->setPreTickFunction(injectedCallback);

  return true;
}

bool Groot2Publisher::unlockBreakpoint(uint16_t node_uid, NodeStatus result, bool remove)
{
  auto it = nodes_by_uid_.find(node_uid);
  if( it == nodes_by_uid_.end())
  {
    return false;
  }
  TreeNode::Ptr node = it->second.lock();
  if(!node)
  {
    return false;
  }

  auto breakpoint = getBreakpoint(node_uid);
  if(!breakpoint)
  {
    return false;
  }

  {
    std::unique_lock<std::mutex> lk(breakpoint->mutex);
    breakpoint->desired_status = result;
    breakpoint->remove_when_done |= remove;
    if(breakpoint->is_interactive)
    {
      breakpoint->ready = true;
      lk.unlock();
      breakpoint->wakeup.notify_all();
    }
  }
  return true;
}

bool Groot2Publisher::removeBreakpoint(uint16_t node_uid)
{
  auto it = nodes_by_uid_.find(node_uid);
  if( it == nodes_by_uid_.end())
  {
    return false;
  }
  TreeNode::Ptr node = it->second.lock();
  if(!node)
  {
    return false;
  }

  auto breakpoint = getBreakpoint(node_uid);
  if(!breakpoint)
  {
    return false;
  }

  {
    std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
    pre_breakpoints_.erase(node_uid);
  }
  node->setPreTickFunction({});

  // Disable breakpoint, if it was interactive and blocked
  {
    std::unique_lock<std::mutex> lk(breakpoint->mutex);
    if(breakpoint->is_interactive)
    {
      breakpoint->enabled = false;
      lk.unlock();
      breakpoint->wakeup.notify_all();
      std::cout << "disable " << node_uid << std::endl;
    }
  }
  return true;
}

void Groot2Publisher::removeAllBreakpoints()
{
  std::vector<uint16_t> uids;
  {
    std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
    for(auto [node_uid, breakpoint]: pre_breakpoints_)
    {
      uids.push_back(node_uid);
    }
  }

  for(auto node_uid: uids)
  {
    removeBreakpoint(node_uid);
  }
}

Monitor::Breakpoint::Ptr Groot2Publisher::getBreakpoint(uint16_t node_uid)
{
    std::unique_lock<std::mutex> lk(breakpoints_map_mutex_);
    auto bk_it = pre_breakpoints_.find(node_uid);
    if( bk_it == pre_breakpoints_.end())
    {
    return {};
    }
    return bk_it->second;
}

}   // namespace BT
