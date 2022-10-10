#include <future>
#include "behaviortree_cpp_v3/loggers/bt_zmq_publisher.h"
#include "behaviortree_cpp_v3/flatbuffers/bt_flatbuffer_helper.h"
#include "cppzmq/zmq.hpp"

namespace BT
{
std::atomic<bool> PublisherZMQ::ref_count(false);

struct PublisherZMQ::Pimpl
{
  Pimpl() : context(1), publisher(context, ZMQ_PUB), server(context, ZMQ_REP)
  {}

  zmq::context_t context;
  zmq::socket_t publisher;
  zmq::socket_t server;
};

PublisherZMQ::PublisherZMQ(const BT::Tree& tree, unsigned max_msg_per_second,
                           unsigned publisher_port, unsigned server_port) :
  StatusChangeLogger(tree.rootNode()),
  tree_(tree),
  min_time_between_msgs_(std::chrono::microseconds(1000 * 1000) / max_msg_per_second),
  send_pending_(false),
  zmq_(new Pimpl())
{
  bool expected = false;
  if (!ref_count.compare_exchange_strong(expected, true))
  {
    throw LogicError("Only one instance of PublisherZMQ shall be created");
  }
  if (publisher_port == server_port)
  {
    throw LogicError("The TCP ports of the publisher and the server must be "
                     "different");
  }

  flatbuffers::FlatBufferBuilder builder(1024);
  CreateFlatbuffersBehaviorTree(builder, tree);

  tree_buffer_.resize(builder.GetSize());
  memcpy(tree_buffer_.data(), builder.GetBufferPointer(), builder.GetSize());

  char str[100];

  sprintf(str, "tcp://*:%d", publisher_port);
  zmq_->publisher.bind(str);
  sprintf(str, "tcp://*:%d", server_port);
  zmq_->server.bind(str);

  int timeout_ms = 100;
  zmq_->server.set(zmq::sockopt::rcvtimeo, timeout_ms);

  active_server_ = true;

  thread_ = std::thread([this]() {
    while (active_server_)
    {
      zmq::message_t req;
      try
      {
        zmq::recv_result_t received = zmq_->server.recv(req);
        if (received)
        {
          zmq::message_t reply(tree_buffer_.size());
          memcpy(reply.data(), tree_buffer_.data(), tree_buffer_.size());
          zmq_->server.send(reply, zmq::send_flags::none);
        }
      }
      catch (zmq::error_t& err)
      {
        if (err.num() == ETERM)
        {
          std::cout << "[PublisherZMQ] Server quitting." << std::endl;
        }
        std::cout << "[PublisherZMQ] just died. Exception " << err.what() << std::endl;
        active_server_ = false;
      }
    }
  });

  createStatusBuffer();
}

PublisherZMQ::~PublisherZMQ()
{
  active_server_ = false;
  if (thread_.joinable())
  {
    thread_.join();
  }
  if (send_pending_)
  {
    send_condition_variable_.notify_all();
    send_future_.get();
  }
  flush();
  zmq_->context.shutdown();
  delete zmq_;
  ref_count = false;
}

void PublisherZMQ::createStatusBuffer()
{
  status_buffer_.clear();
  applyRecursiveVisitor(tree_.rootNode(), [this](TreeNode* node) {
    size_t index = status_buffer_.size();
    status_buffer_.resize(index + 3);
    flatbuffers::WriteScalar<uint16_t>(&status_buffer_[index], node->UID());
    flatbuffers::WriteScalar<int8_t>(
        &status_buffer_[index + 2],
        static_cast<int8_t>(convertToFlatbuffers(node->status())));
  });
}

void PublisherZMQ::callback(Duration timestamp, const TreeNode& node,
                            NodeStatus prev_status, NodeStatus status)
{
  SerializedTransition transition =
      SerializeTransition(node.UID(), timestamp, prev_status, status);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    transition_buffer_.push_back(transition);
  }

  if (!send_pending_.exchange(true))
  {
    send_future_ = std::async(std::launch::async, [this]() {
      std::unique_lock<std::mutex> lock(mutex_);
      const bool is_server_inactive = send_condition_variable_.wait_for(
          lock, min_time_between_msgs_, [this]() { return !active_server_; });
      lock.unlock();
      if (!is_server_inactive)
      {
        flush();
      }
    });
  }
}

void PublisherZMQ::flush()
{
  zmq::message_t message;
  {
    std::unique_lock<std::mutex> lock(mutex_);

    const size_t msg_size = status_buffer_.size() + 8 + (transition_buffer_.size() * 12);

    message.rebuild(msg_size);
    uint8_t* data_ptr = static_cast<uint8_t*>(message.data());

    // first 4 bytes are the side of the header
    flatbuffers::WriteScalar<uint32_t>(data_ptr,
                                       static_cast<uint32_t>(status_buffer_.size()));
    data_ptr += sizeof(uint32_t);
    // copy the header part
    memcpy(data_ptr, status_buffer_.data(), status_buffer_.size());
    data_ptr += status_buffer_.size();

    // first 4 bytes are the side of the transition buffer
    flatbuffers::WriteScalar<uint32_t>(data_ptr,
                                       static_cast<uint32_t>(transition_buffer_.size()));
    data_ptr += sizeof(uint32_t);

    for (auto& transition : transition_buffer_)
    {
      memcpy(data_ptr, transition.data(), transition.size());
      data_ptr += transition.size();
    }
    transition_buffer_.clear();
    createStatusBuffer();
  }
  try
  {
    zmq_->publisher.send(message, zmq::send_flags::none);
  }
  catch (zmq::error_t& err)
  {
    if (err.num() == ETERM)
    {
      std::cout << "[PublisherZMQ] Publisher quitting." << std::endl;
    }
    std::cout << "[PublisherZMQ] just died. Exception " << err.what() << std::endl;
  }

  send_pending_ = false;
  // printf("%.3f zmq send\n", std::chrono::duration<double>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count());
}
}   // namespace BT
