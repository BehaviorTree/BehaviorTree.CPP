#include <future>

#include "behaviortree_cpp_v3/loggers/groot2_publisher.h"
#include "behaviortree_cpp_v3/loggers/groot2_protocol.h"
#include "behaviortree_cpp_v3/xml_parsing.h"
#include "cppzmq/zmq.hpp"
#include "cppzmq/zmq_addon.hpp"

namespace BT
{
//------------------------------------------------------
std::mutex Groot2Publisher::used_ports_mutex;
std::set<unsigned> Groot2Publisher::used_ports;

enum {
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
    publisher.set(zmq::sockopt::rcvtimeo, timeout_rcv);
  }

  unsigned server_port = 0;
  std::string server_address;
  std::string publisher_address;

  std::string tree_xml;

  std::atomic_bool active_server;
  std::thread server_thread;

  std::mutex status_mutex;

  std::string status_buffer;
  // each element of this map points to a character in _p->status_buffer
  std::unordered_map<uint16_t, char*> status_buffermap;

  std::unordered_map<uint16_t, std::weak_ptr<BT::TreeNode>> nodes_by_uid;

  std::atomic_bool recording;
  std::deque<Transition> transitions_buffer;
  std::chrono::microseconds recording_fist_time;

  zmq::context_t context;
  zmq::socket_t server;
  zmq::socket_t publisher;
};

Groot2Publisher::Groot2Publisher(const BT::Tree& tree,
                                 unsigned server_port) :
  StatusChangeLogger(tree.rootNode()),
  _p(new PImpl())
{
  _p->recording = false;
  _p->server_port = server_port;

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

  _p->tree_xml = WriteTreeToXML(tree, true, true);

  //-------------------------------
  // Prepare the status buffer
  _p->status_buffer.resize(3 * tree.nodes.size());

  unsigned ptr_offset = 0;
  char* buffer_ptr = const_cast<char*>(_p->status_buffer.data());

  for (const auto& node : tree.nodes)
  {
    _p->nodes_by_uid.insert({node->UID(), node});

    ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset, node->UID());
    _p->status_buffermap.insert({node->UID(), buffer_ptr + ptr_offset});
    ptr_offset += Monitor::Serialize(buffer_ptr, ptr_offset, uint8_t(NodeStatus::IDLE));
  }

  //-------------------------------
  _p->server_address = StrCat("tcp://*:", std::to_string(server_port));
  _p->publisher_address = StrCat("tcp://*:", std::to_string(server_port+1));

  _p->server.bind(_p->server_address.c_str());
  _p->publisher.bind(_p->publisher_address.c_str());

  _p->server_thread = std::thread(&Groot2Publisher::serverLoop, this);
}

Groot2Publisher::~Groot2Publisher()
{
  _p->active_server = false;
  if (_p->server_thread.joinable())
  {
    _p->server_thread.join();
  }

  flush();

  {
    std::unique_lock<std::mutex> lk(Groot2Publisher::used_ports_mutex);
    Groot2Publisher::used_ports.erase(_p->server_port);
  }
}

void Groot2Publisher::callback(Duration ts, const TreeNode& node,
                               NodeStatus prev_status, NodeStatus new_status)
{
  std::unique_lock<std::mutex> lk(_p->status_mutex);
  auto status = static_cast<char>(new_status);

  if( new_status == NodeStatus::IDLE) {
    status = 10 + static_cast<char>(prev_status);
  }
  *(_p->status_buffermap.at(node.UID())) = status;

  if(_p->recording)
  {
    Transition trans;
    trans.node_uid = node.UID();
    trans.status = static_cast<uint8_t>(new_status);
    auto timestamp = ts -_p->recording_fist_time;
    trans.timestamp_usec =
        std::chrono::duration_cast<std::chrono::microseconds>(timestamp).count();
    _p->transitions_buffer.push_back(trans);
    while(_p->transitions_buffer.size() > 1000) {
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

  auto sendErrorReply = [&socket](const std::string& msg)
  {
    zmq::multipart_t error_msg;
    error_msg.addstr("error");
    error_msg.addstr(msg);
    error_msg.send(socket);
  };

  while (_p->active_server)
  {
    zmq::multipart_t requestMsg;
    if( !requestMsg.recv(socket) || requestMsg.size() == 0)
    {
      continue;
    }

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
    reply_msg.addstr( Monitor::SerializeHeader(reply_header) );

    switch(request_header.type)
    {
      case Monitor::RequestType::FULLTREE:
      {
        reply_msg.addstr( _p->tree_xml );
      } break;

      case Monitor::RequestType::STATUS:
      {
        std::unique_lock<std::mutex> lk(_p->status_mutex);
        reply_msg.addstr( _p->status_buffer );
      } break;

      case Monitor::RequestType::BLACKBOARD:
      case Monitor::RequestType::HOOK_INSERT:
      case Monitor::RequestType::BREAKPOINT_UNLOCK:
      case Monitor::RequestType::REMOVE_ALL_HOOKS:
      case Monitor::RequestType::DISABLE_ALL_HOOKS:
      case Monitor::RequestType::HOOK_REMOVE:
      case Monitor::RequestType::HOOKS_DUMP:
      {
        sendErrorReply("not_supported");
        continue;
      }

      case Monitor::RequestType::TOGGLE_RECORDING:
      {
        if(requestMsg.size() != 2) {
          sendErrorReply("must be 2 parts message");
          continue;
        }

        auto const cmd = (requestMsg[1].to_string());
        if(cmd == "start")
        {
          _p->recording = true;
          auto now = std::chrono::system_clock::now();

          _p->recording_fist_time = std::chrono::duration_cast<std::chrono::microseconds>
                                   (now.time_since_epoch());

          reply_msg.addstr(std::to_string(_p->recording_fist_time.count()));
          std::unique_lock<std::mutex> lk(_p->status_mutex);
          _p->transitions_buffer.clear();
        }
        else if(cmd == "stop")
        {
          _p->recording = false;
        }
      } break;

      case Monitor::RequestType::GET_TRANSITIONS:
      {
        thread_local std::string trans_buffer;
        trans_buffer.resize(9 * _p->transitions_buffer.size());

        std::unique_lock<std::mutex> lk(_p->status_mutex);
        size_t offset = 0;
        for(const auto& trans: _p->transitions_buffer)
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

}   // namespace BT
