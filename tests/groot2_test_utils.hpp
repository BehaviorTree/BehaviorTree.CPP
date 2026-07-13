#pragma once

#include <cerrno>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

#include <behaviortree_cpp/loggers/groot2_protocol.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>

#include <zmq_addon.hpp>

namespace Groot2Test
{
struct PublisherAndPort
{
  std::unique_ptr<BT::Groot2Publisher> publisher;
  unsigned port = 0;
};

inline PublisherAndPort makePublisher(const BT::Tree& tree)
{
  std::random_device random_device;
  std::uniform_int_distribution<unsigned> port_distribution(20000, 60000);

  for(size_t attempt = 0; attempt < 100; ++attempt)
  {
    // Groot2Publisher also binds port + 1. Using an even base avoids overlap
    // between candidates and makes accidental clashes less likely.
    const unsigned port = port_distribution(random_device) & ~1U;
    try
    {
      return { std::make_unique<BT::Groot2Publisher>(tree, port), port };
    }
    catch(const zmq::error_t& error)
    {
      if(error.num() != EADDRINUSE)
      {
        throw;
      }
    }
  }
  throw std::runtime_error("Could not find two free ports for Groot2Publisher");
}

class Client
{
public:
  explicit Client(unsigned port) : socket_(context_, ZMQ_REQ)
  {
    socket_.set(zmq::sockopt::linger, 0);
    socket_.set(zmq::sockopt::rcvtimeo, 2000);
    socket_.set(zmq::sockopt::sndtimeo, 2000);
    socket_.connect("tcp://127.0.0.1:" + std::to_string(port));
  }

  zmq::multipart_t rawRequest(BT::Monitor::RequestType type,
                              std::optional<std::string> payload = std::nullopt)
  {
    const BT::Monitor::RequestHeader header(type);
    zmq::multipart_t request;
    request.addstr(BT::Monitor::SerializeHeader(header));
    if(payload)
    {
      request.addstr(*payload);
    }
    if(!request.send(socket_))
    {
      throw std::runtime_error("Timed out sending a Groot2 request");
    }

    zmq::multipart_t reply;
    if(!reply.recv(socket_))
    {
      throw std::runtime_error("Timed out receiving a Groot2 reply");
    }
    return reply;
  }

  zmq::multipart_t request(BT::Monitor::RequestType type,
                           std::optional<std::string> payload = std::nullopt)
  {
    auto reply = rawRequest(type, std::move(payload));
    if(reply.empty() || reply[0].size() != BT::Monitor::ReplyHeader::size())
    {
      throw std::runtime_error("Received an invalid Groot2 reply");
    }
    return reply;
  }

private:
  zmq::context_t context_;
  zmq::socket_t socket_;
};
}  // namespace Groot2Test
