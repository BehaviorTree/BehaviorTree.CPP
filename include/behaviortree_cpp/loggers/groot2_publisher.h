#pragma once

#include <array>
#include <future>
#include "behaviortree_cpp/loggers/abstract_logger.h"

namespace BT
{
class Groot2Publisher : public StatusChangeLogger
{
  static std::mutex used_ports_mutex;
  static std::set<unsigned> used_ports;

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

  void breakpointsLoop();

  void updateStatusBuffer();

  std::vector<uint8_t> generateBlackboardsDump(const std::string& bb_list);

  bool insertBreakpoint(uint16_t node_uid, bool once);

  bool removeBreakpoint(uint16_t node_uid);

  unsigned server_port_ = 0;
  std::string server_address_;
  std::string client_address_;

  std::string tree_xml_;

  std::atomic_bool active_server_;
  std::thread server_thread_;

  std::mutex status_mutex_;

  std::unordered_map<uint16_t, char*> buffer_ptr_;
  std::string status_buffer_;

  // weak reference to the tree.
  std::unordered_map<std::string, std::weak_ptr<BT::Tree::Subtree>> subtrees_;
  std::unordered_map<uint16_t, std::weak_ptr<BT::TreeNode>> nodes_by_uid_;

  struct Breakpoint
  {
    int node_uid = -1;
    std::condition_variable wakeup;
    std::mutex mutex;
    bool ready = false;
    bool remove_when_done = false;
    NodeStatus desired_result = NodeStatus::SKIPPED;
  };

  std::unordered_map<int, std::shared_ptr<Breakpoint>> pre_breakpoints_;

  std::chrono::system_clock::time_point last_heartbeat_;

  std::thread breakpoints_thread_;
  std::deque<std::shared_ptr<Breakpoint>> breakpoints_queue_;
  std::condition_variable breakpoints_cv_;
  std::mutex breakpoints_mutex_;

  struct Pimpl;
  Pimpl* zmq_;
};
}   // namespace BT

