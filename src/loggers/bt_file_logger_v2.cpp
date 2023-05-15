#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/flatbuffers/base.h"
#include "behaviortree_cpp/xml_parsing.h"

namespace BT
{

int64_t ToUsec(Duration ts)
{
  return std::chrono::duration_cast<std::chrono::microseconds>(ts).count();
}

FileLogger2::FileLogger2(const BT::Tree& tree, std::filesystem::path const& filepath) :
  StatusChangeLogger(tree.rootNode())
{
  enableTransitionToIdle(true);

  //-------------------------------------
  file_stream_.open(filepath, std::ofstream::binary | std::ofstream::out);

  std::string const xml = WriteTreeToXML(tree, true);

  // serialize the length of the buffer in the first 4 bytes
  char write_buffer[8];
  flatbuffers::WriteScalar(write_buffer, static_cast<int32_t>(xml.size()));
  file_stream_.write(write_buffer, 4);

  // write the XML definition
  file_stream_.write(xml.data(), int(xml.size()));

  first_timestamp_ = std::chrono::system_clock::now().time_since_epoch();

  // save the first timestamp in the next 8 bytes (microseconds)
  int64_t timestamp_usec = ToUsec(first_timestamp_);
  flatbuffers::WriteScalar(write_buffer, timestamp_usec);
  file_stream_.write(write_buffer, 8);

  writer_thread_ = std::thread(&FileLogger2::writerLoop, this);
}

FileLogger2::~FileLogger2()
{
  loop_ = false;
  queue_cv_.notify_one();
  writer_thread_.join();
  file_stream_.close();
}

void FileLogger2::callback(Duration timestamp, const TreeNode& node,
                          NodeStatus /*prev_status*/, NodeStatus status)
{
  Transition trans;
  trans.timestamp_usec = uint64_t(ToUsec(timestamp - first_timestamp_));
  trans.node_uid = node.UID();
  trans.status = static_cast<uint64_t>(status);
  {
    std::scoped_lock lock(queue_mutex_);
    transitions_queue_.push_back(trans);
  }
  queue_cv_.notify_one();
}

void FileLogger2::flush()
{
  file_stream_.flush();
}

void FileLogger2::writerLoop()
{
  // local buffer in this thread
  std::deque<Transition> transitions;

  while(loop_)
  {
    transitions.clear();
    {
      std::unique_lock lock(queue_mutex_);
      queue_cv_.wait_for(lock, std::chrono::milliseconds(10),
                         [this]() {return !transitions_queue_.empty() && loop_; } );
      // simple way to pop all the transitions from transitions_queue_ into transitions
      std::swap(transitions, transitions_queue_);
    }
    while(!transitions.empty())
    {
      char write_buffer[8];
      flatbuffers::WriteScalar(write_buffer, transitions.front());
      file_stream_.write(write_buffer, 8);
      transitions.pop_front();
    }
    file_stream_.flush();
  }
}

}   // namespace BT
