#ifndef BT_COUT_LOGGER_H
#define BT_COUT_LOGGER_H

#include <cstring>
#include "behaviortree_cpp/loggers/abstract_logger.h"

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

  virtual void flush() override;

private:
  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;
};

}  // namespace BT

#endif  // BT_COUT_LOGGER_H
