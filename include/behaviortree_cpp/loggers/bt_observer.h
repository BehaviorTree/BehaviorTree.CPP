#ifndef BT_OBSERVER_H
#define BT_OBSERVER_H

#include <cstring>
#include "abstract_logger.h"

namespace BT
{

class TreeObserver : public StatusChangeLogger
{
public:
  TreeObserver(const BT::Tree& tree);
  ~TreeObserver() override;

  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;

  virtual void flush() override {}

  void resetStatistics();

  struct NodeStatistics
  {
    NodeStatus last_result = NodeStatus::IDLE;
    NodeStatus current_status = NodeStatus::IDLE;
    unsigned success_count = 0;
    unsigned failure_count = 0;
    unsigned tick_count = 0;
    Duration last_timestamp = {};
  };

  // find the statistics of a node, based on its path
  const NodeStatistics& getStatistics(const std::string& path) const;

  // find the statistics of a node, based on its TreeNode::UID()
  const NodeStatistics& getStatistics(uint16_t uid) const;

  // all statistics
  const std::unordered_map<uint16_t, NodeStatistics>& statistics() const;

  // path to UID map
  const std::unordered_map<std::string, uint16_t>& pathToUID() const;

  private:
  std::unordered_map<uint16_t, NodeStatistics> _statistics;
  std::unordered_map<std::string, uint16_t> _path_to_uid;
};

}   // namespace BT

#endif   // BT_OBSERVER_H
