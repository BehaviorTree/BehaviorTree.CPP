#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "flatbuffers/base.h"

namespace BT
{

int64_t ToUsec(Duration ts)
{
  return std::chrono::duration_cast<std::chrono::microseconds>(ts).count();
}

struct FileLogger2::PImpl
{
  std::ofstream file_stream;

  Duration first_timestamp = {};

  std::deque<Transition> transitions_queue;
  std::condition_variable queue_cv;
  std::mutex queue_mutex;

  std::thread writer_thread;
  std::atomic_bool loop = true;
};

FileLogger2::FileLogger2(const BT::Tree& tree, std::filesystem::path const& filepath)
  : StatusChangeLogger(tree.rootNode()), _p(new PImpl)
{
  if(filepath.filename().extension() != ".btlog")
  {
    throw RuntimeError("FileLogger2: the file extension must be [.btlog]");
  }

  enableTransitionToIdle(true);

  //-------------------------------------
  _p->file_stream.open(filepath, std::ofstream::binary | std::ofstream::out);

  if(!_p->file_stream.is_open())
  {
    throw RuntimeError("problem opening file in FileLogger2");
  }

  _p->file_stream << "BTCPP4-FileLogger2";

  const uint8_t protocol = 1;

  _p->file_stream << protocol;

  std::string const xml = WriteTreeToXML(tree, true, true);

  // serialize the length of the buffer in the first 4 bytes
  char write_buffer[8];
  flatbuffers::WriteScalar(write_buffer, static_cast<int32_t>(xml.size()));
  _p->file_stream.write(write_buffer, 4);

  // write the XML definition
  _p->file_stream.write(xml.data(), int(xml.size()));

  _p->first_timestamp = std::chrono::system_clock::now().time_since_epoch();

  // save the first timestamp in the next 8 bytes (microseconds)
  int64_t timestamp_usec = ToUsec(_p->first_timestamp);
  flatbuffers::WriteScalar(write_buffer, timestamp_usec);
  _p->file_stream.write(write_buffer, 8);

  _p->writer_thread = std::thread(&FileLogger2::writerLoop, this);
}

FileLogger2::~FileLogger2()
{
  _p->loop = false;
  _p->queue_cv.notify_one();
  _p->writer_thread.join();
  _p->file_stream.close();
}

void FileLogger2::callback(Duration timestamp, const TreeNode& node,
                           NodeStatus /*prev_status*/, NodeStatus status)
{
  Transition trans;
  trans.timestamp_usec = uint64_t(ToUsec(timestamp - _p->first_timestamp));
  trans.node_uid = node.UID();
  trans.status = static_cast<uint64_t>(status);
  {
    std::scoped_lock lock(_p->queue_mutex);
    _p->transitions_queue.push_back(trans);
  }
  _p->queue_cv.notify_one();
}

void FileLogger2::flush()
{
  _p->file_stream.flush();
}

void FileLogger2::writerLoop()
{
  // local buffer in this thread
  std::deque<Transition> transitions;

  while(_p->loop)
  {
    transitions.clear();
    {
      std::unique_lock lock(_p->queue_mutex);
      _p->queue_cv.wait_for(lock, std::chrono::milliseconds(10), [this]() {
        return !_p->transitions_queue.empty() && _p->loop;
      });
      // simple way to pop all the transitions from _p->transitions_queue into transitions
      std::swap(transitions, _p->transitions_queue);
    }
    while(!transitions.empty())
    {
      const auto trans = transitions.front();
      std::array<char, 9> write_buffer;
      std::memcpy(write_buffer.data(), &trans.timestamp_usec, 6);
      std::memcpy(write_buffer.data() + 6, &trans.node_uid, 2);
      std::memcpy(write_buffer.data() + 8, &trans.status, 1);

      _p->file_stream.write(write_buffer.data(), 9);
      transitions.pop_front();
    }
    _p->file_stream.flush();
  }
}

}  // namespace BT
