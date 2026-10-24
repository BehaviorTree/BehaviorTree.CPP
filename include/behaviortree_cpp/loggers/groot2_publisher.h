#pragma once

#include "behaviortree_cpp/loggers/abstract_logger.h"
#include "behaviortree_cpp/loggers/groot2_protocol.h"

#include <array>
#include <future>

namespace BT
{

/**
 * @brief The Groot2Publisher is used to create an interface between
 * your BT.CPP executor and Groot2.
 *
 * An inter-process communication mechanism allows the two processes
 * to communicate through a TCP port. The user should provide the
 * port to be used in the constructor. The publisher binds two ports:
 * server_port and server_port + 1.
 *
 * SECURITY: the protocol has no authentication. Any client that can reach
 * these ports can read the tree and its blackboards, and can insert hooks
 * (breakpoints) that replace the result of a node, changing how the tree
 * executes. Do not expose them to untrusted networks; use the bind_address
 * argument to restrict the interface they listen on.
 */
class Groot2Publisher : public StatusChangeLogger
{
  using Position = Monitor::Hook::Position;

public:
  /// Listens on all network interfaces.
  Groot2Publisher(const BT::Tree& tree, unsigned server_port = 1667);

  /**
   * @param bind_address the interface to listen on, in ZeroMQ tcp syntax:
   * an IPv4 address ("127.0.0.1" for local connections only), an
   * interface name ("eth0"), or "*" for all interfaces.
   */
  Groot2Publisher(const BT::Tree& tree, unsigned server_port,
                  const std::string& bind_address);

  ~Groot2Publisher() override;

  Groot2Publisher(const Groot2Publisher& other) = delete;
  Groot2Publisher& operator=(const Groot2Publisher& other) = delete;

  Groot2Publisher(Groot2Publisher&& other) = delete;
  Groot2Publisher& operator=(Groot2Publisher&& other) = delete;

  /**
   * @brief setMaxHeartbeatDelay is used to tell the publisher
   * when a connection with Groot2 should be cancelled, if no
   * heartbeat is received.
   *
   * Default is 5000 ms
   */
  void setMaxHeartbeatDelay(std::chrono::milliseconds delay);

  std::chrono::milliseconds maxHeartbeatDelay() const;

private:
  void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                NodeStatus status) override;

  void flush() override;

  void serverLoop();

  void heartbeatLoop();

  void updateStatusBuffer();

  Expected<std::vector<uint8_t>> generateBlackboardsDump(const std::string& bb_list);

  bool insertHook(Monitor::Hook::Ptr breakpoint);

  bool unlockBreakpoint(Position pos, uint16_t node_uid, NodeStatus result, bool remove);

  bool removeHook(Position pos, uint16_t node_uid);

  void removeAllHooks();

  Monitor::Hook::Ptr getHook(Position pos, uint16_t node_uid);

  struct PImpl;
  std::unique_ptr<PImpl> _p;

  void enableAllHooks(bool enable);
};
}  // namespace BT
