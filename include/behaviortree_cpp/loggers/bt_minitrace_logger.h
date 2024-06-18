#pragma once

#include "behaviortree_cpp/loggers/abstract_logger.h"

namespace BT
{
class MinitraceLogger : public StatusChangeLogger
{
public:
  MinitraceLogger(const BT::Tree& tree, const char* filename_json);

  virtual ~MinitraceLogger() override;

  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;

  virtual void flush() override;

private:
  TimePoint prev_time_;
};

}  // namespace BT
