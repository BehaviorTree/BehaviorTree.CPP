#pragma once

#include <filesystem>
#include "behaviortree_cpp/loggers/abstract_logger.h"

// forward declaration
struct sqlite3;

namespace BT
{

/** SQL schema
 *
 * CREATE TABLE IF NOT EXISTS Definitions (
 *     session_id INTEGER PRIMARY KEY AUTOINCREMENT,
 *     date       TEXT NOT NULL,
 *     xml_tree   TEXT NOT NULL);
 *
 * CREATE TABLE IF NOT EXISTS Nodes ("
 *     session_id INTEGER NOT NULL,
 *     fullpath   VARCHAR, "
 *     node_uid   INTEGER NOT NULL );
 *
 * CREATE TABLE IF NOT EXISTS Transitions (
 *     timestamp  INTEGER PRIMARY KEY NOT NULL,
 *     session_id INTEGER NOT NULL,
 *     node_uid   INTEGER NOT NULL,
 *     duration   INTEGER,
 *     state      INTEGER NOT NULL,
 *     extra_data VARCHAR );
 *
 */

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
  /**
   * @brief To correctly read this log with Groot2, you must use the suffix ".db3".
   * Constructor will throw otherwise.
   *
   * @param tree      the tree to log
   * @param filepath  path of the file where info will be stored
   * @param append    if true, add this recording to the database
   */
  SqliteLogger(const Tree& tree, std::filesystem::path const& file, bool append = false);

  virtual ~SqliteLogger() override;

  // You can inject a function that add a string to the Transitions table,
  // in the column "extra_data".
  // The arguments of the function are the same as SqliteLogger::callback()
  using ExtraCallback =
      std::function<std::string(Duration, const TreeNode&, NodeStatus, NodeStatus)>;
  void setAdditionalCallback(ExtraCallback func);

  virtual void callback(Duration timestamp, const TreeNode& node, NodeStatus prev_status,
                        NodeStatus status) override;

  void execSqlStatement(std::string statement);

  virtual void flush() override;

private:
  sqlite3* db_ = nullptr;

  int64_t monotonic_timestamp_ = 0;
  std::unordered_map<const BT::TreeNode*, int64_t> starting_time_;

  int session_id_ = -1;

  struct Transition
  {
    uint16_t node_uid;
    int64_t timestamp;
    int64_t duration;
    NodeStatus status;
    std::string extra_data;
  };

  std::deque<Transition> transitions_queue_;
  std::condition_variable queue_cv_;
  std::mutex queue_mutex_;

  std::thread writer_thread_;
  std::atomic_bool loop_ = true;

  ExtraCallback extra_func_;

  void writerLoop();
};

}  // namespace BT
