#pragma once
#include <fstream>
#include <deque>
#include <array>
#include <filesystem>
#include "behaviortree_cpp/loggers/abstract_logger.h"

namespace BT
{
/**
 * @brief The FileLogger2 is a logger that saves the tree as
 * XML and all the transitions.
 * Data is written to file in a separate thread, to minimize latency.
 *
 * Format:
 *
 * - first 4 bytes: size of the XML string (N)
 * - next N bytes: string containing the XML representing the tree.
 * - next 8 bytes: first timestamp (microseconds since epoch)
 * - next: each 9 bytes is a FileLogger2::Transition. See definition.
 *
 */
class FileLogger2 : public StatusChangeLogger
{
public:
  /**
   * @brief To correctly read this log with Groot2, you must use the suffix ".btlog".
   * Constructor will throw otherwise.
   *
   * @param tree      the tree to log
   * @param filepath  path of the file where info will be stored
   */
  FileLogger2(const Tree& tree, std::filesystem::path const& filepath);

  FileLogger2(const FileLogger2& other) = delete;
  FileLogger2& operator=(const FileLogger2& other) = delete;

  FileLogger2(FileLogger2&& other) = default;
  FileLogger2& operator=(FileLogger2&& other) = default;

  virtual ~FileLogger2() override;

  void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                NodeStatus status) override;

  struct Transition
  {
    // when serializing, we will remove the initial time and serialize only
    // 6 bytes, instead of 8
    uint64_t timestamp_usec;
    // if you have more than 64.000 nodes, you are doing something wrong :)
    uint16_t node_uid;
    // enough bits to contain NodeStatus
    uint8_t status;
  };

  void flush() override;

private:
  struct PImpl;
  std::unique_ptr<PImpl> _p;

  void writerLoop();
};

}  // namespace BT
