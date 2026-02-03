/* Copyright (C) 2018-2025 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/loggers/bt_minitrace_logger.h"
#include "behaviortree_cpp/loggers/bt_sqlite_logger.h"

#include <cstdio>
#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace BT;

class LoggerTest : public testing::Test
{
protected:
  BT::BehaviorTreeFactory factory;
  std::string test_dir;

  void SetUp() override
  {
    // Create a temporary directory for test files
    test_dir = std::filesystem::temp_directory_path().string() + "/bt_logger_test";
    std::filesystem::create_directories(test_dir);
  }

  void TearDown() override
  {
    // Clean up test files
    std::filesystem::remove_all(test_dir);
  }

  BT::Tree createSimpleTree()
  {
    const std::string xml_text = R"(
      <root BTCPP_format="4">
         <BehaviorTree>
            <Sequence>
              <AlwaysSuccess name="ActionA"/>
              <AlwaysSuccess name="ActionB"/>
            </Sequence>
         </BehaviorTree>
      </root>)";
    return factory.createTreeFromText(xml_text);
  }
};

// ============ StdCoutLogger tests ============

TEST_F(LoggerTest, StdCoutLogger_Creation)
{
  auto tree = createSimpleTree();

  // Should not throw
  StdCoutLogger logger(tree);

  // Execute the tree - logger should capture transitions
  tree.tickWhileRunning();

  // Flush should not throw
  logger.flush();
}

TEST_F(LoggerTest, StdCoutLogger_Enabled)
{
  auto tree = createSimpleTree();
  StdCoutLogger logger(tree);

  // Test enabled/disabled
  ASSERT_TRUE(logger.enabled());

  logger.setEnabled(false);
  ASSERT_FALSE(logger.enabled());

  logger.setEnabled(true);
  ASSERT_TRUE(logger.enabled());

  tree.tickWhileRunning();
}

TEST_F(LoggerTest, StdCoutLogger_TransitionToIdle)
{
  auto tree = createSimpleTree();
  StdCoutLogger logger(tree);

  // Test transition to idle setting
  ASSERT_TRUE(logger.showsTransitionToIdle());

  logger.enableTransitionToIdle(false);
  ASSERT_FALSE(logger.showsTransitionToIdle());

  logger.enableTransitionToIdle(true);
  ASSERT_TRUE(logger.showsTransitionToIdle());

  tree.tickWhileRunning();
}

TEST_F(LoggerTest, StdCoutLogger_TimestampType)
{
  auto tree = createSimpleTree();
  StdCoutLogger logger(tree);

  // Test timestamp type setting (default is absolute)
  logger.setTimestampType(TimestampType::relative);
  tree.tickWhileRunning();

  logger.setTimestampType(TimestampType::absolute);
  tree.haltTree();
  tree.tickWhileRunning();
}

// ============ FileLogger2 tests ============

TEST_F(LoggerTest, FileLogger2_Creation)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/test.btlog";

  {
    FileLogger2 logger(tree, filepath);
    tree.tickWhileRunning();
    logger.flush();
  }  // Logger destructor closes the file

  // File should exist
  ASSERT_TRUE(std::filesystem::exists(filepath));

  // File should have content
  auto file_size = std::filesystem::file_size(filepath);
  ASSERT_GT(file_size, 0u);
}

TEST_F(LoggerTest, FileLogger2_WrongExtension)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/test.txt";

  // Should throw with wrong extension
  ASSERT_THROW(FileLogger2 logger(tree, filepath), RuntimeError);
}

TEST_F(LoggerTest, FileLogger2_FileHeader)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/header_test.btlog";

  {
    FileLogger2 logger(tree, filepath);
    tree.tickWhileRunning();
  }

  // Read the file and check header
  std::ifstream file(filepath, std::ios::binary);
  ASSERT_TRUE(file.is_open());

  char header[18];
  file.read(header, 18);
  std::string header_str(header, 18);
  ASSERT_EQ(header_str, "BTCPP4-FileLogger2");
}

TEST_F(LoggerTest, FileLogger2_MultipleTicks)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/multi_tick.btlog";

  {
    FileLogger2 logger(tree, filepath);

    // Multiple tick cycles
    for(int i = 0; i < 3; i++)
    {
      tree.tickWhileRunning();
      tree.haltTree();
    }
    logger.flush();
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}

// ============ MinitraceLogger tests ============

TEST_F(LoggerTest, MinitraceLogger_Creation)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/trace.json";

  {
    MinitraceLogger logger(tree, filepath.c_str());
    tree.tickWhileRunning();
    logger.flush();
  }  // Logger destructor flushes and shuts down

  // File should exist
  ASSERT_TRUE(std::filesystem::exists(filepath));

  // File should have content
  auto file_size = std::filesystem::file_size(filepath);
  ASSERT_GT(file_size, 0u);
}

TEST_F(LoggerTest, MinitraceLogger_JsonContent)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/trace_content.json";

  {
    MinitraceLogger logger(tree, filepath.c_str());
    tree.tickWhileRunning();
  }

  // Read file and check it's valid JSON-ish (starts with proper content)
  std::ifstream file(filepath);
  ASSERT_TRUE(file.is_open());

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  // Minitrace JSON should contain traceEvents
  ASSERT_NE(content.find("traceEvents"), std::string::npos);
}

TEST_F(LoggerTest, MinitraceLogger_TransitionTypes)
{
  // Test different node types are logged correctly
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence name="SeqNode">
            <Inverter name="InvNode">
              <AlwaysFailure name="FailNode"/>
            </Inverter>
            <AlwaysSuccess name="SuccessNode"/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  std::string filepath = test_dir + "/trace_types.json";

  {
    MinitraceLogger logger(tree, filepath.c_str());
    logger.enableTransitionToIdle(true);
    tree.tickWhileRunning();
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}

// ============ SqliteLogger tests ============

TEST_F(LoggerTest, SqliteLogger_Creation_db3)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/test.db3";

  {
    SqliteLogger logger(tree, filepath);
    tree.tickWhileRunning();
    logger.flush();
  }

  // File should exist
  ASSERT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(LoggerTest, SqliteLogger_Creation_btdb)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/test.btdb";

  {
    SqliteLogger logger(tree, filepath);
    tree.tickWhileRunning();
    logger.flush();
  }

  // File should exist
  ASSERT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(LoggerTest, SqliteLogger_WrongExtension)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/test.sqlite";

  // Should throw with wrong extension
  ASSERT_THROW(SqliteLogger logger(tree, filepath), RuntimeError);
}

TEST_F(LoggerTest, SqliteLogger_AppendMode)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/append_test.db3";

  // First session
  {
    SqliteLogger logger(tree, filepath, false);
    tree.tickWhileRunning();
  }

  auto size_after_first = std::filesystem::file_size(filepath);

  // Second session with append=true
  {
    SqliteLogger logger(tree, filepath, true);
    tree.tickWhileRunning();
  }

  auto size_after_second = std::filesystem::file_size(filepath);
  ASSERT_GT(size_after_second, size_after_first);
}

TEST_F(LoggerTest, SqliteLogger_NoAppendMode)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/noappend_test.db3";

  // First session
  {
    SqliteLogger logger(tree, filepath, false);
    tree.tickWhileRunning();
  }

  // Second session with append=false should clear data
  {
    SqliteLogger logger(tree, filepath, false);
    tree.tickWhileRunning();
  }

  // File should still exist
  ASSERT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(LoggerTest, SqliteLogger_ExtraCallback)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/extra_callback.db3";

  int callback_count = 0;
  {
    SqliteLogger logger(tree, filepath);

    logger.setAdditionalCallback([&callback_count](Duration, const TreeNode&, NodeStatus,
                                                   NodeStatus) -> std::string {
      callback_count++;
      return "extra_data";
    });

    tree.tickWhileRunning();
  }

  ASSERT_GT(callback_count, 0);
}

TEST_F(LoggerTest, SqliteLogger_ExecSqlStatement)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/exec_sql.db3";

  {
    SqliteLogger logger(tree, filepath);
    tree.tickWhileRunning();

    // Execute a custom SQL statement
    logger.execSqlStatement("SELECT COUNT(*) FROM Transitions;");
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(LoggerTest, SqliteLogger_MultipleTicks)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/multi_tick.db3";

  {
    SqliteLogger logger(tree, filepath);

    for(int i = 0; i < 5; i++)
    {
      tree.tickWhileRunning();
      tree.haltTree();
    }
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}

// ============ Multiple loggers simultaneously ============

TEST_F(LoggerTest, MultipleLoggers)
{
  auto tree = createSimpleTree();

  std::string btlog_path = test_dir + "/multi.btlog";
  std::string json_path = test_dir + "/multi.json";
  std::string db_path = test_dir + "/multi.db3";

  {
    StdCoutLogger cout_logger(tree);
    FileLogger2 file_logger(tree, btlog_path);
    MinitraceLogger trace_logger(tree, json_path.c_str());
    SqliteLogger sql_logger(tree, db_path);

    tree.tickWhileRunning();

    cout_logger.flush();
    file_logger.flush();
    trace_logger.flush();
    sql_logger.flush();
  }

  // All files should exist
  ASSERT_TRUE(std::filesystem::exists(btlog_path));
  ASSERT_TRUE(std::filesystem::exists(json_path));
  ASSERT_TRUE(std::filesystem::exists(db_path));
}

// ============ Logger with async actions ============

TEST_F(LoggerTest, LoggerWithAsyncTree)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence>
            <Sleep msec="10" name="Sleep1"/>
            <AlwaysSuccess name="Action"/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  std::string filepath = test_dir + "/async.db3";

  {
    SqliteLogger logger(tree, filepath);
    tree.tickWhileRunning();
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}

// ============ Edge cases ============

TEST_F(LoggerTest, Logger_EmptyTree)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <AlwaysSuccess/>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  std::string filepath = test_dir + "/empty.db3";

  {
    SqliteLogger logger(tree, filepath);
    tree.tickWhileRunning();
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(LoggerTest, Logger_DisabledDuringExecution)
{
  auto tree = createSimpleTree();
  std::string filepath = test_dir + "/disabled.db3";

  {
    SqliteLogger logger(tree, filepath);
    logger.setEnabled(false);
    tree.tickWhileRunning();
    logger.setEnabled(true);
    tree.haltTree();
    tree.tickWhileRunning();
  }

  ASSERT_TRUE(std::filesystem::exists(filepath));
}
