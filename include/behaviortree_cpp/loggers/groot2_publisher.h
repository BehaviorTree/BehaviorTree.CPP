#pragma once

#include <array>
#include <future>
#include "behaviortree_cpp/loggers/abstract_logger.h"
#include "behaviortree_cpp/loggers/groot2_protocol.h"

namespace BT
{

/**
 * @brief The Groot2Publisher is used to create an interface between
 * your BT.CPP executor and Groot2.
 *
 * An inter-process communication mechanism allows the two processes
 * to communicate through a TCP port. The user should provide the
 * port to be used in the constructor.
 */
class Groot2Publisher : public StatusChangeLogger
{
  static std::mutex used_ports_mutex;
  static std::set<unsigned> used_ports;

  using Position = Monitor::Hook::Position;

public:
  Groot2Publisher(const BT::Tree& tree, unsigned server_port = 1667);

  ~Groot2Publisher() override;

  Groot2Publisher(const Groot2Publisher& other) = delete;
  Groot2Publisher& operator=(const Groot2Publisher& other) = delete;

  Groot2Publisher(Groot2Publisher&& other) = default;
  Groot2Publisher& operator=(Groot2Publisher&& other) = default;

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

  std::vector<uint8_t> generateBlackboardsDump(const std::string& bb_list);

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
