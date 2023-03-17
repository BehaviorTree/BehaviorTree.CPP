#pragma once

#include <array>
#include <future>
#include "behaviortree_cpp/loggers/abstract_logger.h"
#include "behaviortree_cpp/loggers/groot2_protocol.h"

namespace BT
{

class Groot2Publisher : public StatusChangeLogger
{
  static std::mutex used_ports_mutex;
  static std::set<unsigned> used_ports;

  using Position = Monitor::Hook::Position;

  public:
  Groot2Publisher(const BT::Tree& tree, unsigned server_port = 1667);

  ~Groot2Publisher() override;

  private:

  void callback(Duration timestamp,
                const TreeNode& node,
                NodeStatus prev_status,
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

  unsigned server_port_ = 0;
  std::string server_address_;
  std::string publisher_address_;

  std::string tree_xml_;

  std::atomic_bool active_server_;
  std::thread server_thread_;

  std::mutex status_mutex_;

  std::unordered_map<uint16_t, char*> buffer_ptr_;
  std::string status_buffer_;

  // weak reference to the tree.
  std::unordered_map<std::string, std::weak_ptr<BT::Tree::Subtree>> subtrees_;
  std::unordered_map<uint16_t, std::weak_ptr<BT::TreeNode>> nodes_by_uid_;

  std::mutex hooks_map_mutex_;
  std::unordered_map<uint16_t, Monitor::Hook::Ptr> pre_hooks_;
  std::unordered_map<uint16_t, Monitor::Hook::Ptr> post_hooks_;

  std::chrono::system_clock::time_point last_heartbeat_;

  std::thread heartbeat_thread_;

  struct Pimpl;
  Pimpl* zmq_;
  void enableAllHooks(bool enable);
};
}   // namespace BT

