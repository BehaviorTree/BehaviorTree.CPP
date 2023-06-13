#ifndef BT_FILE_LOGGER_H
#define BT_FILE_LOGGER_H

#include <fstream>
#include <deque>
#include <array>
#include "behaviortree_cpp/loggers/abstract_logger.h"
#include "behaviortree_cpp/flatbuffers/bt_flatbuffer_helper.h"

namespace BT
{

class [[deprecated("Use FileLogger2 instead")]]
FileLogger : public StatusChangeLogger
{
public:
  FileLogger(const Tree& tree, const char* filename, uint16_t buffer_size = 10);

  virtual ~FileLogger() override;

  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;

  virtual void flush() override;

private:
  std::ofstream file_os_;

  std::chrono::high_resolution_clock::time_point start_time;

  std::vector<SerializedTransition> buffer_;

  size_t buffer_max_size_;
};

}   // namespace BT

#endif   // BT_FILE_LOGGER_H
