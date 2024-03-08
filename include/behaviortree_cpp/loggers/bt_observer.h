#ifndef BT_OBSERVER_H
#define BT_OBSERVER_H

#include <cstring>
#include "behaviortree_cpp/loggers/abstract_logger.h"

namespace BT
{

/**
 * @brief The TreeObserver is used to collect statistics about which nodes
 * are executed and their returned status.
 *
 * It is particularly useful to create unit tests, since if allow to
 * determine if a certain transition happened as expected, in a non intrusive way.
 */
class TreeObserver : public StatusChangeLogger
{
public:
  TreeObserver(const BT::Tree& tree);
  ~TreeObserver() override;

  virtual void flush() override
  {}

  void resetStatistics();

  struct NodeStatistics
  {
    // Last __valid__ result, either SUCCESS or FAILURE
    NodeStatus last_result = NodeStatus::IDLE;
    // Last status. Can be any status, including IDLE or SKIPPED
    NodeStatus current_status = NodeStatus::IDLE;

    // count status transitions, excluding transition to IDLE
    unsigned transitions_count = 0;
    // count number of transitions to SUCCESS
    unsigned success_count = 0;
    // count number of transitions to FAILURE
    unsigned failure_count = 0;
    // count number of transitions to SKIPPED
    unsigned skip_count = 0;

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

  const std::map<uint16_t, std::string>& uidToPath() const;

private:
  std::unordered_map<uint16_t, NodeStatistics> _statistics;
  std::unordered_map<std::string, uint16_t> _path_to_uid;
  std::map<uint16_t, std::string> _uid_to_path;

  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;
};

}  // namespace BT

#endif  // BT_OBSERVER_H
