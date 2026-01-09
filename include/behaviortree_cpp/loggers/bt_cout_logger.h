#ifndef BT_COUT_LOGGER_H
#define BT_COUT_LOGGER_H

#include "behaviortree_cpp/loggers/abstract_logger.h"

#include <cstring>

namespace BT
{
/**
 * @brief StdCoutLogger is a very simple logger that
 * displays all the transitions on the console.
 */

class StdCoutLogger : public StatusChangeLogger
{
public:
  StdCoutLogger(const BT::Tree& tree);
  ~StdCoutLogger() override;

  StdCoutLogger(const StdCoutLogger&) = delete;
  StdCoutLogger& operator=(const StdCoutLogger&) = delete;
  StdCoutLogger(StdCoutLogger&&) = delete;
  StdCoutLogger& operator=(StdCoutLogger&&) = delete;

  virtual void flush() override;

private:
  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;
};

}  // namespace BT

#endif  // BT_COUT_LOGGER_H
