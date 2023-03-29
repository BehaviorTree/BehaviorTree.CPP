#pragma once

#include <filesystem>
#include "behaviortree_cpp/loggers/abstract_logger.h"

namespace sqlite{
class Connection;
}

namespace BT
{

/**
 * @brief The SqliteLogger is a logger that will store the tree and all the
 * status transitions in a SQLite database (single file).
 *
 * You can append data to the same file; this allows you to store multiple experiments into the database.
 * Yn that case, each recording has a unique session_id.
 *
 * This is primarily meant to be used with Groot2, but the content of
 * the tables is sufficiently self-explaining, and you can create
 * your own tools to extract the information.
 */
class SqliteLogger : public StatusChangeLogger
{
public:
  SqliteLogger(const Tree &tree,
               std::filesystem::path const& file,
               bool append = false);

  virtual ~SqliteLogger() override;

  virtual void callback(Duration timestamp,
                        const TreeNode& node,
                        NodeStatus prev_status,
                        NodeStatus status) override;

  virtual void flush() override;

private:
  std::unique_ptr<sqlite::Connection> db_;

  int64_t monotonic_timestamp_ = 0;
  std::unordered_map<const BT::TreeNode*, int64_t> starting_time_;

  int session_id_ = -1;

  struct Transition {
    uint16_t node_uid;
    int64_t timestamp;
    int64_t duration;
    NodeStatus status;
  };

  std::deque<Transition> write_queue_;
  std::condition_variable queue_push_cv_;
  std::condition_variable queue_pop_cv_;
  std::mutex queue_mutex_;

  std::thread queue_thread_;
  std::atomic_bool loop_ = true;

  void threadLoop();

};

}

