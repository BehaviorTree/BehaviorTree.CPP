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
  {}

  zmq::context_t context;
  zmq::socket_t server;
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

  // copy of the subtrees
  for(auto& subtree: tree.subtrees) {
    subtrees_.insert( {subtree->instance_name, subtree} );
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
    for(const auto& node: subtree->nodes)
    {
      ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset,
                                       node->UID());
      buffer_ptr_.insert( {node->UID(), buffer_ptr + ptr_offset} );
      ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset,
                                       uint8_t(NodeStatus::IDLE));
    }
  }
  //-------------------------------
  server_address_ = StrCat("tcp://*:", std::to_string(server_port));
  zmq_->server.bind(server_address_.c_str());

  zmq_->server.set(zmq::sockopt::linger, 0);

  int timeout_rcv = 100;
  zmq_->server.set(zmq::sockopt::rcvtimeo, timeout_rcv);

  int timeout_ms = 1000;
  zmq_->server.set(zmq::sockopt::sndtimeo, timeout_ms);

  thread_ = std::thread(&Groot2Publisher::threadLoop, this);
}

Groot2Publisher::~Groot2Publisher()
{
  active_server_ = false;
  if (thread_.joinable())
  {
    thread_.join();
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

void Groot2Publisher::threadLoop()
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
          zmq::message_t error_msg(std::string("must be 2 parts message"));
          socket.send(error_msg, zmq::send_flags::none);
          break;
        }
        std::string const bb_names_str = requestMsg[1].to_string();
        auto msg = generateBlackboardsDump(bb_names_str);
        reply_msg.addmem(msg.data(), msg.size());
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

}   // namespace BT
