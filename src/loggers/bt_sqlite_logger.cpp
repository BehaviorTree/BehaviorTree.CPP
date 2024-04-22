#include "behaviortree_cpp/loggers/bt_sqlite_logger.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "cpp-sqlite/sqlite.hpp"

namespace BT
{

SqliteLogger::SqliteLogger(const Tree& tree, std::filesystem::path const& filepath,
                           bool append)
  : StatusChangeLogger(tree.rootNode())
{
  const auto extension = filepath.filename().extension();
  if(extension != ".db3" && extension != ".btdb")
  {
    throw RuntimeError("SqliteLogger: the file extension must be [.db3] or [.btdb]");
  }

  enableTransitionToIdle(true);

  db_ = std::make_unique<sqlite::Connection>(filepath.string());

  sqlite::Statement(*db_, "CREATE TABLE IF NOT EXISTS Transitions ("
                          "timestamp  INTEGER PRIMARY KEY NOT NULL, "
                          "session_id INTEGER NOT NULL, "
                          "node_uid   INTEGER NOT NULL, "
                          "duration   INTEGER, "
                          "state      INTEGER NOT NULL,"
                          "extra_data VARCHAR );");

  sqlite::Statement(*db_, "CREATE TABLE IF NOT EXISTS Nodes ("
                          "session_id INTEGER NOT NULL, "
                          "fullpath   VARCHAR, "
                          "node_uid   INTEGER NOT NULL );");

  sqlite::Statement(*db_, "CREATE TABLE IF NOT EXISTS Definitions ("
                          "session_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "date       TEXT NOT NULL,"
                          "xml_tree   TEXT NOT NULL);");

  if(!append)
  {
    sqlite::Statement(*db_, "DELETE from Transitions;");
    sqlite::Statement(*db_, "DELETE from Definitions;");
    sqlite::Statement(*db_, "DELETE from Nodes;");
  }

  auto tree_xml = WriteTreeToXML(tree, true, true);
  sqlite::Statement(*db_,
                    "INSERT into Definitions (date, xml_tree) "
                    "VALUES (datetime('now','localtime'),?);",
                    tree_xml);

  auto res = sqlite::Query(*db_, "SELECT MAX(session_id) "
                                 "FROM Definitions LIMIT 1;");

  while(res.Next())
  {
    session_id_ = res.Get(0);
  }

  for(const auto& subtree : tree.subtrees)
  {
    for(const auto& node : subtree->nodes)
    {
      sqlite::Statement(*db_, "INSERT INTO Nodes VALUES (?, ?, ?)", session_id_,
                        node->fullPath(), node->UID());
    }
  }

  writer_thread_ = std::thread(&SqliteLogger::writerLoop, this);
}

SqliteLogger::~SqliteLogger()
{
  loop_ = false;
  queue_cv_.notify_one();
  writer_thread_.join();
  flush();
  sqlite::Statement(*db_, "PRAGMA optimize;");
}

void SqliteLogger::setAdditionalCallback(ExtraCallback func)
{
  extra_func_ = func;
}

void SqliteLogger::callback(Duration timestamp, const TreeNode& node,
                            NodeStatus prev_status, NodeStatus status)
{
  using namespace std::chrono;
  int64_t tm_usec = int64_t(duration_cast<microseconds>(timestamp).count());
  monotonic_timestamp_ = std::max(monotonic_timestamp_ + 1, tm_usec);

  long elapsed_time = 0;

  if(prev_status == NodeStatus::IDLE && status == NodeStatus::RUNNING)
  {
    starting_time_[&node] = monotonic_timestamp_;
  }

  if(prev_status == NodeStatus::RUNNING && status != NodeStatus::RUNNING)
  {
    elapsed_time = monotonic_timestamp_;
    auto it = starting_time_.find(&node);
    if(it != starting_time_.end())
    {
      elapsed_time -= it->second;
    }
  }

  Transition trans;
  trans.timestamp = monotonic_timestamp_;
  trans.duration = elapsed_time;
  trans.node_uid = node.UID();
  trans.status = status;

  if(extra_func_)
  {
    trans.extra_data = extra_func_(timestamp, node, prev_status, status);
  }

  {
    std::scoped_lock lk(queue_mutex_);
    transitions_queue_.push_back(trans);
  }
  queue_cv_.notify_one();

  if(extra_func_)
  {
    extra_func_(timestamp, node, prev_status, status);
  }
}

void SqliteLogger::execSqlStatement(std::string statement)
{
  sqlite::Statement(*db_, statement);
}

void SqliteLogger::writerLoop()
{
  std::deque<Transition> transitions;

  while(loop_)
  {
    transitions.clear();
    {
      std::unique_lock lk(queue_mutex_);
      queue_cv_.wait(lk, [this]() { return !transitions_queue_.empty() || !loop_; });
      std::swap(transitions, transitions_queue_);
    }

    while(!transitions.empty())
    {
      auto const trans = transitions.front();
      transitions.pop_front();

      sqlite::Statement(*db_, "INSERT INTO Transitions VALUES (?, ?, ?, ?, ?, ?)",
                        trans.timestamp, session_id_, trans.node_uid, trans.duration,
                        static_cast<int>(trans.status), trans.extra_data);
    }
  }
}

void BT::SqliteLogger::flush()
{
  sqlite3_db_cacheflush(db_->GetPtr());
}

}  // namespace BT
