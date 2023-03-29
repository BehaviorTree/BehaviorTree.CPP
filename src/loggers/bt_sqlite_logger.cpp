#include "behaviortree_cpp/loggers/bt_sqlite_logger.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "cpp-sqlite/sqlite.hpp"

namespace BT {

SqliteLogger::SqliteLogger(const Tree &tree,
                           std::filesystem::path const& file,
                           bool append):
  StatusChangeLogger(tree.rootNode())
{
  enableTransitionToIdle(true);

  db_ = std::make_unique<sqlite::Connection>(file.string());

  sqlite::Statement(*db_, "PRAGMA journal_mode=WAL;");
  sqlite::Statement(*db_, "PRAGMA synchronous = normal;");
  sqlite::Statement(*db_, "PRAGMA temp_store = memory;");

  sqlite::Statement(*db_,
                    "CREATE TABLE IF NOT EXISTS Transitions ("
                    "timestamp  INTEGER PRIMARY KEY NOT NULL, "
                    "session_id INTEGER NOT NULL, "
                    "uid        INTEGER NOT NULL, "
                    "duration   INTEGER, "
                    "state      INTEGER NOT NULL);");

  sqlite::Statement(*db_,
                    "CREATE TABLE IF NOT EXISTS Definitions ("
                    "session_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "date       TEXT NOT NULL,"
                    "xml_tree   TEXT NOT NULL);");

  if( !append )
  {
    sqlite::Statement(*db_, "DELETE from Transitions;");
    sqlite::Statement(*db_, "DELETE from Definitions;");
  }

  auto tree_xml = WriteTreeToXML(tree, true);
  sqlite::Statement(*db_,
                    "INSERT into Definitions (date, xml_tree) "
                    "VALUES (datetime('now','localtime'),?);",
                    tree_xml);

  auto res = sqlite::Query(*db_, "SELECT MAX(session_id) FROM Definitions LIMIT 1;");

  while(res.Next())
  {
    session_id_ = res.Get(0);
  }
  queue_thread_ = std::thread(&SqliteLogger::threadLoop, this);
}

SqliteLogger::~SqliteLogger()
{
  flush();
  loop_ = false;
  queue_push_cv_.notify_one();
  queue_thread_.join();
  sqlite::Statement(*db_, "PRAGMA optimize;");
}

void SqliteLogger::callback(Duration timestamp,
                            const TreeNode &node,
                            NodeStatus prev_status,
                            NodeStatus status)
{

  using namespace std::chrono;
  int64_t tm_usec = int64_t(duration_cast<microseconds>(timestamp).count());
  monotonic_timestamp_ = std::max( monotonic_timestamp_ + 1, tm_usec);

  long elapsed_time = 0;

  if( prev_status == NodeStatus::IDLE && status == NodeStatus::RUNNING )
  {
    starting_time_[&node] = monotonic_timestamp_;
  }

  if( prev_status == NodeStatus::RUNNING && status != NodeStatus::RUNNING )
  {
    elapsed_time = monotonic_timestamp_;
    auto it = starting_time_.find(&node);
    if( it != starting_time_.end() )
    {
      elapsed_time -= it->second;
    }
  }

  Transition trans;
  trans.timestamp = monotonic_timestamp_;
  trans.duration = elapsed_time;
  trans.node_uid = node.UID();
  trans.status = status;

  {
    std::scoped_lock lk(queue_mutex_);
    write_queue_.push_back(trans);
  }
  queue_push_cv_.notify_one();
}

void SqliteLogger::threadLoop()
{
  while(loop_)
  {
    std::unique_lock lk(queue_mutex_);
    queue_push_cv_.wait(lk, [this]() {
      return !write_queue_.empty() || !loop_;
    });

    if(!loop_) {
      break;
    }

    auto const trans = write_queue_.front();
    write_queue_.pop_front();
    lk.unlock();
    queue_pop_cv_.notify_all();

    sqlite::Statement(
        *db_,
        "INSERT INTO Transitions VALUES (?, ?, ?, ?, ?)",
        trans.timestamp,
        session_id_,
        trans.node_uid,
        trans.duration,
        static_cast<int>(trans.status));
  }
}

void BT::SqliteLogger::flush()
{
  std::unique_lock lk(queue_mutex_);
  queue_pop_cv_.wait(lk, [&]() {
    return write_queue_.empty();
  });
}

}
