#include "behaviortree_cpp/loggers/bt_sqlite_logger.h"
#include "behaviortree_cpp/xml_parsing.h"
#include <sqlite3.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace BT
{

namespace
{
// Helper function to execute a SQL statement and check for errors
void execSQL(sqlite3* db, const std::string& sql)
{
  char* err_msg = nullptr;
  const int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
  if(rc != SQLITE_OK)
  {
    std::string error = "SQL error: ";
    if(err_msg != nullptr)
    {
      error += err_msg;
      sqlite3_free(err_msg);
    }
    throw RuntimeError(error);
  }
}

// Helper function to prepare a statement
sqlite3_stmt* prepareStatement(sqlite3* db, const std::string& sql)
{
  sqlite3_stmt* stmt = nullptr;
  const int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
  if(rc != SQLITE_OK)
  {
    throw RuntimeError(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
  }
  return stmt;
}

// Helper function to execute a prepared statement
void execStatement(sqlite3_stmt* stmt)
{
  const int rc = sqlite3_step(stmt);
  if(rc != SQLITE_DONE && rc != SQLITE_ROW)
  {
    throw RuntimeError(std::string("Failed to execute statement: ") + std::to_string(rc));
  }
  sqlite3_finalize(stmt);
}

}  // namespace

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

  // Open database
  const int rc = sqlite3_open(filepath.string().c_str(), &db_);
  if(rc != SQLITE_OK)
  {
    throw RuntimeError(std::string("Cannot open database: ") + sqlite3_errmsg(db_));
  }

  // Create tables
  execSQL(db_, "CREATE TABLE IF NOT EXISTS Transitions ("
               "timestamp  INTEGER PRIMARY KEY NOT NULL, "
               "session_id INTEGER NOT NULL, "
               "node_uid   INTEGER NOT NULL, "
               "duration   INTEGER, "
               "state      INTEGER NOT NULL,"
               "extra_data VARCHAR );");

  execSQL(db_, "CREATE TABLE IF NOT EXISTS Nodes ("
               "session_id INTEGER NOT NULL, "
               "fullpath   VARCHAR, "
               "node_uid   INTEGER NOT NULL );");

  execSQL(db_, "CREATE TABLE IF NOT EXISTS Definitions ("
               "session_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "date       TEXT NOT NULL,"
               "xml_tree   TEXT NOT NULL);");

  if(!append)
  {
    execSQL(db_, "DELETE from Transitions;");
    execSQL(db_, "DELETE from Definitions;");
    execSQL(db_, "DELETE from Nodes;");
  }

  // Insert tree definition
  auto tree_xml = WriteTreeToXML(tree, true, true);
  sqlite3_stmt* stmt = prepareStatement(db_, "INSERT into Definitions (date, xml_tree) "
                                             "VALUES (datetime('now','localtime'),?);");
  sqlite3_bind_text(stmt, 1, tree_xml.c_str(), -1, SQLITE_TRANSIENT);
  execStatement(stmt);

  // Get session_id
  stmt = prepareStatement(db_, "SELECT MAX(session_id) FROM Definitions LIMIT 1;");
  if(sqlite3_step(stmt) == SQLITE_ROW)
  {
    session_id_ = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);

  // Insert nodes
  for(const auto& subtree : tree.subtrees)
  {
    for(const auto& node : subtree->nodes)
    {
      stmt = prepareStatement(db_, "INSERT INTO Nodes VALUES (?, ?, ?)");
      sqlite3_bind_int(stmt, 1, session_id_);
      sqlite3_bind_text(stmt, 2, node->fullPath().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 3, node->UID());
      execStatement(stmt);
    }
  }

  writer_thread_ = std::thread(&SqliteLogger::writerLoop, this);
}

SqliteLogger::~SqliteLogger()
{
  try
  {
    loop_ = false;
    queue_cv_.notify_one();
    writer_thread_.join();
    flush();
    execSQL(db_, "PRAGMA optimize;");
  }
  catch(const std::exception& ex)
  {
    std::cerr << "Exception in ~SqliteLogger(): " << ex.what() << std::endl;
  }
  sqlite3_close(db_);
}

void SqliteLogger::setAdditionalCallback(ExtraCallback func)
{
  extra_func_ = func;
}

void SqliteLogger::callback(Duration timestamp, const TreeNode& node,
                            NodeStatus prev_status, NodeStatus status)
{
  using namespace std::chrono;
  const int64_t tm_usec = int64_t(duration_cast<microseconds>(timestamp).count());
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
    const std::scoped_lock lk(queue_mutex_);
    transitions_queue_.push_back(trans);
  }
  queue_cv_.notify_one();
}

void SqliteLogger::execSqlStatement(std::string statement)
{
  execSQL(db_, statement);
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

      sqlite3_stmt* stmt = prepareStatement(db_, "INSERT INTO Transitions VALUES (?, ?, "
                                                 "?, ?, ?, ?)");
      sqlite3_bind_int64(stmt, 1, trans.timestamp);
      sqlite3_bind_int(stmt, 2, session_id_);
      sqlite3_bind_int(stmt, 3, trans.node_uid);
      sqlite3_bind_int64(stmt, 4, trans.duration);
      sqlite3_bind_int(stmt, 5, static_cast<int>(trans.status));
      sqlite3_bind_text(stmt, 6, trans.extra_data.c_str(), -1, SQLITE_TRANSIENT);
      execStatement(stmt);
    }
  }
}

void BT::SqliteLogger::flush()
{
  sqlite3_db_cacheflush(db_);
}

}  // namespace BT
