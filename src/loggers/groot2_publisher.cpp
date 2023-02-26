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
  Pimpl() : context(), server(context, ZMQ_REP)
  {
  }

  void createClient(const std::string& address)
  {
    client = std::make_unique<zmq::socket_t>(context, ZMQ_REQ);
    client->set(zmq::sockopt::linger, 0);

    int timeout_rcv = 100;
    client->set(zmq::sockopt::rcvtimeo, timeout_rcv);
    int timeout_ms = 1000;
    client->set(zmq::sockopt::sndtimeo, timeout_ms);
    client->connect(address.c_str());
  }

  zmq::context_t context;
  zmq::socket_t server;
  std::unique_ptr<zmq::socket_t> client;
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
  client_address_ = StrCat("tcp://*:", std::to_string(server_port+1));

  zmq_->server.bind(server_address_.c_str());
  zmq_->server.set(zmq::sockopt::linger, 0);
  int timeout_rcv = 100;
  zmq_->server.set(zmq::sockopt::rcvtimeo, timeout_rcv);
  int timeout_ms = 1000;
  zmq_->server.set(zmq::sockopt::sndtimeo, timeout_ms);

  zmq_->createClient(client_address_);

  server_thread_ = std::thread(&Groot2Publisher::serverLoop, this);
  breakpoints_thread_ = std::thread(&Groot2Publisher::breakpointsLoop, this);
}

Groot2Publisher::~Groot2Publisher()
{
  active_server_ = false;
  if (server_thread_.joinable())
  {
    server_thread_.join();
  }

  if (breakpoints_thread_.joinable())
  {
    breakpoints_thread_.join();
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
    if(request_str.size() != 6)
    {
      zmq::message_t error_msg(std::string("error"));
      socket.send(error_msg, zmq::send_flags::none);
      std::cout << "Groot2Publisher: Wrong request size" << std::endl;
      continue;
    }

    auto request_header = Monitor::DeserializeRequestHeader(request_str);

    Monitor::ReplyHeader reply_header;
    reply_header.request = request_header;
    reply_header.tree_id = serialized_uuid;

    zmq::multipart_t reply_msg;
    reply_msg.addstr( Monitor::SerializeHeader(reply_header) );

    auto sendErrorReply = [this, &socket](const std::string& msg)
    {
      zmq::message_t error_msg(msg);
      socket.send(error_msg, zmq::send_flags::none);
    };

    switch(request_header.type)
    {
      case Monitor::RequestType::FULLTREE: {
        reply_msg.addstr( tree_xml_ );
      } break;

      case Monitor::RequestType::STATUS: {
        std::unique_lock<std::mutex> lk(status_mutex_);
        reply_msg.addstr( status_buffer_ );
      } break;

      case Monitor::RequestType::BLACKBOARD: {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          break;
        }
        std::string const bb_names_str = requestMsg[1].to_string();
        auto msg = generateBlackboardsDump(bb_names_str);
        reply_msg.addmem(msg.data(), msg.size());
      } break;

      case Monitor::RequestType::BREAKPOINT_INSERT: {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          break;
        }
        auto str_parts = splitString(requestMsg[1].to_string_view(), ';');
        int node_uid = std::stoi( std::string(str_parts[0]) );
        bool once = str_parts.size() >= 2 && str_parts[1] == "once";

        if(!insertBreakpoint(uint16_t(node_uid), once))
        {
          sendErrorReply("Node ID not found");
          break;
        }
      } break;

      case Monitor::RequestType::BREAKPOINT_REMOVE: {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          break;
        }
        int node_uid = std::stoi( requestMsg[1].to_string() );
        if(!removeBreakpoint(uint16_t(node_uid)))
        {
          sendErrorReply("Node ID not found");
          break;
        }
      } break;

      default: {
        zmq::message_t error_msg(std::string("error"));
        socket.send(error_msg, zmq::send_flags::none);
        continue;
      }
    }
    // send the reply
    reply_msg.send(socket);
  }
}

void Groot2Publisher::breakpointsLoop()
{
  auto& groot_socket = *zmq_->client;
  while (active_server_)
  {
    std::shared_ptr<Breakpoint> breakpoint;
    // pop the pending breakpoint request from breakpoints_queue_
    {
      std::unique_lock lk(breakpoints_mutex_);
      // wait for something to be available on
      breakpoints_cv_.wait_for(lk, std::chrono::milliseconds(100),
                               [this](){ return !breakpoints_queue_.empty(); } );
      breakpoint = breakpoints_queue_.front();
      breakpoints_queue_.pop_front();
    }

    if(!breakpoint)
    {
      // empty queue
      continue;
    }

    auto unlockBreakpoint = [breakpoint]()
    {
      {
        std::scoped_lock lk(breakpoint->mutex);
        breakpoint->ready = true;
        breakpoint->desired_result = NodeStatus::SKIPPED;
      }
      breakpoint->wakeup.notify_all();
    };

    // send a request to Groot and wait for reply.
    Monitor::RequestHeader breakpoint_request(Monitor::BREAKPOINT_QUERY);
    zmq::multipart_t request_msg;
    request_msg.addstr( Monitor::SerializeHeader(breakpoint_request) );
    request_msg.addstr(std::to_string(breakpoint->node_uid));
    request_msg.send(groot_socket);

    // wait for the reply, but in case of missing heartbeat, unlock
    // and destroy the breakpoint.
    zmq::multipart_t reply_msg;
    while( !reply_msg.recv(groot_socket) || reply_msg.size() == 0)
    {
      auto const now = std::chrono::system_clock::now();
      if(now - last_heartbeat_ > std::chrono::milliseconds(15000))
      {
        // timeout!!! Destroy the breakpoint and reset the socket
        breakpoint->remove_when_done = true;
        breakpoint->desired_result = NodeStatus::SKIPPED;
        unlockBreakpoint();
        // this is needed to cleanup the internal state of the socket
        zmq_->createClient(client_address_);
        break;
      }
    }
    // valid replies have 2 parts
    if(reply_msg.size() == 2)
    {
      auto str_parts = splitString(reply_msg[1].to_string_view(), ';');

      if( str_parts[0] == "SUCCESS")
      {
        breakpoint->desired_result = NodeStatus::SUCCESS;
      }
      else if( str_parts[0] == "FAILURE")
      {
        breakpoint->desired_result = NodeStatus::FAILURE;
      }
      else {
        breakpoint->desired_result = NodeStatus::SKIPPED;
      }
      if(str_parts.size()>=2 && str_parts[1] == "remove" )
      {
        breakpoint->remove_when_done = true;
      }
      unlockBreakpoint();
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

bool Groot2Publisher::insertBreakpoint(uint16_t node_uid, bool once)
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

  auto breakpoint = std::make_shared<Breakpoint>();
  breakpoint->node_uid = node_uid;
  breakpoint->remove_when_done = once;
  pre_breakpoints_[node_uid] = breakpoint;

  node->setPreTickFunction(
      [breakpoint, this](TreeNode& node) -> NodeStatus
      {
        // push the breakpoint pointer into the breakpoints_queue_
        {
          std::scoped_lock lk(breakpoints_mutex_);
          breakpoints_queue_.push_back(breakpoint);
        }
        // notify to thread in breakpointsLoop() that something was pushed into the queue
        breakpoints_cv_.notify_all();

        // wait until someone wake us up
        std::unique_lock lk(breakpoint->mutex);
        breakpoint->wakeup.wait(lk, [breakpoint]() { return breakpoint->ready; } );
        breakpoint->ready = false;

        // self-destruction at the end of this lambda function
        if(breakpoint->remove_when_done)
        {
          pre_breakpoints_.erase(breakpoint->node_uid);
          node.setPreTickFunction({});
        }
        return breakpoint->desired_result;
      });

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

  auto bk_it = pre_breakpoints_.find(node_uid);
  if( bk_it == pre_breakpoints_.end())
  {
    return false;
  }

  auto breakpoint = bk_it->second;

  // Unlock breakpoint, just in case
  {
    std::scoped_lock lk(breakpoint->mutex);
    breakpoint->desired_result = NodeStatus::SKIPPED;
    breakpoint->ready = true;
    breakpoint->remove_when_done = true;
  }
  breakpoint->wakeup.notify_all();
  node->setPreTickFunction({});
  pre_breakpoints_.erase(bk_it);

  return true;
}

}   // namespace BT
