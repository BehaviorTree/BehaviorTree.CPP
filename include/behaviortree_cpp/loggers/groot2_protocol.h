#pragma once

#include <cstdint>
#include <array>
#include <cstring>
#include <stdexcept>
#include <random>
#include <memory>
#include <condition_variable>
#include <mutex>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/utils/json.hpp"

namespace BT::Monitor {

/*
 * All the messages exchange with the BT executor are multipart ZMQ request-replies.
 *
 * The first part of the request and the reply have fixed size and are described below.
 * The request and reply must have the same value of the fields:
 *
 *  - request_id
 *  - request_type
 *  - protocol_id
 */

enum RequestType : uint8_t
{
  // Request the entire tree defintion as XML
  FULLTREE = 'T',
  // Request the staus of all the nodes
  STATUS = 'S',
  // retrieve the valus in a set of blackboards
  BLACKBOARD = 'B',

  // Groot requests the insertion of a breakpoint
  BREAKPOINT_INSERT = 'I',
  // Groot requests to remove a breakpoint
  BREAKPOINT_REMOVE = 'R',
  // Notify Groot that we reached a breakpoint
  BREAKPOINT_REACHED = 'N',
  // Groot will unlock a breakpoint
  BREAKPOINT_UNLOCK = 'U',
  // receive the existing breakpoints in JSON format
  BREAKPOINTS_DUMP = 'D',

  // Remove all breakpoints. To be done before disconnecting Groot
  REMOVE_ALL_BREAKPOINTS = 'A',

  DISABLE_ALL_BREAKPOINTS = 'X',

  UNDEFINED = 0,
};

inline const char* ToString(const RequestType& type)
{
  switch(type)
  {
  case RequestType::FULLTREE: return "full_tree";
  case RequestType::STATUS: return "status";
  case RequestType::BLACKBOARD: return "blackboard";

  case RequestType::BREAKPOINT_INSERT: return "breakpoint_insert";
  case RequestType::BREAKPOINT_REMOVE: return "breakpoint_remove";
  case RequestType::BREAKPOINT_REACHED: return "breakpoint_reached";
  case RequestType::BREAKPOINT_UNLOCK: return "breakpoint_unlock";
  case RequestType::REMOVE_ALL_BREAKPOINTS: return "breakpoint_remove_all";
  case RequestType::BREAKPOINTS_DUMP: return "breakpoints_dump";
  case RequestType::DISABLE_ALL_BREAKPOINTS: return "disable_breakpoints";

  case RequestType::UNDEFINED: return "undefined";
  }
  return "undefined";
}

constexpr uint8_t kProtocolID = 1;
using TreeUniqueUUID = std::array<char, 16>;

struct RequestHeader
{
  uint32_t unique_id = 0;
  uint8_t protocol = kProtocolID;
  RequestType type = RequestType::UNDEFINED;

  static size_t size() {
    return sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t);
  }

  RequestHeader() = default;

  RequestHeader(RequestType type): type(type)
  {
    // a random number for request_id will do
    static std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint32_t> dist;
    unique_id = dist(mt);
  }

  bool operator==(const RequestHeader& other) const
  {
    return type == other.type &&
           protocol == other.protocol &&
           unique_id == other.unique_id;
  }
  bool operator!=(const RequestHeader& other) const
  {
    return !(*this == other);
  }
};

struct ReplyHeader
{
  RequestHeader request;
  TreeUniqueUUID tree_id;

  static size_t size() {
    return RequestHeader::size() + 16;
  }

  ReplyHeader() {
    tree_id.fill(0);
  }
};

template <typename T> inline
unsigned Serialize(char* buffer, unsigned offset, T value)
{
  memcpy(buffer + offset, &value, sizeof(T));
  return sizeof(T);
}

template <typename T> inline
    unsigned Deserialize(const char* buffer, unsigned offset, T& value)
{
  memcpy(reinterpret_cast<char*>(&value), buffer + offset, sizeof(T));
  return sizeof(T);
}


inline std::string SerializeHeader(const RequestHeader& header)
{
  std::string buffer;
  buffer.resize(6);
  unsigned offset = 0;
  offset += Serialize(buffer.data(), offset, header.protocol);
  offset += Serialize(buffer.data(), offset, uint8_t(header.type));
  offset += Serialize(buffer.data(), offset, header.unique_id);
  return buffer;
}

inline std::string SerializeHeader(const ReplyHeader& header)
{
  // copy the first part directly (6 bytes)
  std::string buffer = SerializeHeader(header.request);
  // add the following 16 bytes
  unsigned const offset = 6;
  buffer.resize(offset + 16);
  Serialize(buffer.data(), offset, header.tree_id);
  return buffer;
}

inline RequestHeader DeserializeRequestHeader(const std::string& buffer)
{
  RequestHeader header;
  unsigned offset = 0;
  offset += Deserialize(buffer.data(), offset, header.protocol);
  uint8_t type;
  offset += Deserialize(buffer.data(), offset, type);
  header.type = static_cast<Monitor::RequestType>(type);
  offset += Deserialize(buffer.data(), offset, header.unique_id);
  return header;
}


inline ReplyHeader DeserializeReplyHeader(const std::string& buffer)
{
  ReplyHeader header;
  header.request = DeserializeRequestHeader(buffer);
  unsigned const offset = 6;
  Deserialize(buffer.data(), offset, header.tree_id);
  return header;
}

struct Breakpoint
{
  using Ptr = std::shared_ptr<Breakpoint>;

  // used to enable/disable the breakpoint
  bool enabled = true;

  uint16_t node_uid = 0;

  // interactive breakpoints are unblucked using unlockBreakpoint()
  bool is_interactive = true;

  // used by interactive breakpoints to wait for unlocking
  std::condition_variable wakeup;

  std::mutex mutex;

  // set to true to unlock an interactive breakpoint
  bool ready = false;

  // once finished self-destroy
  bool remove_when_done = false;

  // result to be returned
  NodeStatus desired_status = NodeStatus::SKIPPED;
};


void to_json(nlohmann::json& js, const Breakpoint& bp) {
  js = nlohmann::json {
      {"enabled", bp.enabled},
      {"uid", bp.node_uid},
      {"interactive", bp.is_interactive},
      {"once", bp.remove_when_done},
      {"desired_status", toStr(bp.desired_status)}
  };
}

void from_json(const nlohmann::json& js, Breakpoint& bp) {
  js.at("enabled").get_to(bp.enabled);
  js.at("uid").get_to(bp.node_uid);
  js.at("interactive").get_to(bp.is_interactive);
  js.at("once").get_to(bp.remove_when_done);
  const std::string desired_value = js.at("desired_status").get<std::string>();
  bp.desired_status = convertFromString<NodeStatus>(desired_value);
}

}
