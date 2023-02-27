#pragma once

#include <cstdint>
#include <array>
#include <cstring>
#include <stdexcept>
#include <random>

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
  BREAKPOINT_NOTIFY = 'N',
  // Groot will unlock a breakpoint
  BREAKPOINT_UNLOCK = 'U',

  // Remove all breakpoints. To be done before disconnecting Groot
  REMOVE_ALL_BREAKPOINTS = 'A',

  UNDEFINED = 0,
};

inline const char* ToString(const RequestType& type)
{
  switch(type)
  {
  case RequestType::FULLTREE: return "FullTree";
  case RequestType::STATUS: return "Status";
  case RequestType::BLACKBOARD: return "BlackBoard";

  case RequestType::BREAKPOINT_INSERT: return "BreakpointInsert";
  case RequestType::BREAKPOINT_REMOVE: return "BreakpointRemove";
  case RequestType::BREAKPOINT_NOTIFY: return "BreakpointNotify";
  case RequestType::BREAKPOINT_UNLOCK: return "BreakpointUnlock";
  case RequestType::REMOVE_ALL_BREAKPOINTS: return "BreakpointRemoveAll";

  case RequestType::UNDEFINED: return "UNDEFINED";
  }
  return "Undefined";
}


constexpr uint8_t kProtocolID = 1;
using TreeUniqueUUID = std::array<char, 16>;

struct RequestHeader
{
  uint32_t unique_id = 0;
  uint8_t protocol = kProtocolID;
  RequestType type = RequestType::UNDEFINED;

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

}
