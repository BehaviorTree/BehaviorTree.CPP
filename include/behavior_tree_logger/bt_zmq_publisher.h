#ifndef BT_ZMQ_PUBLISHER_H
#define BT_ZMQ_PUBLISHER_H

#include <array>
#include <future>
#include <zmq.hpp>
#include "abstract_logger.h"
#include "BT_logger_generated.h"

namespace BT{

class PublisherZMQ: public StatusChangeLogger
{
public:
    PublisherZMQ(TreeNode* root_node, int max_msg_per_second = 25);

    virtual ~PublisherZMQ();

private:

    virtual void callback(TimePoint timestamp,
                          const TreeNode& node,
                          NodeStatus prev_status, NodeStatus status) override;

    virtual void flush() override;

    TreeNode *_root_node;
    std::vector<uint8_t> _tree_buffer;
    std::vector<uint8_t> _status_buffer;
    std::vector< std::array<uint8_t,12> > _transition_buffer;
    std::chrono::microseconds _min_time_between_msgs;

    zmq::context_t _zmq_context;
    zmq::socket_t  _zmq_publisher;
    zmq::socket_t  _zmq_server;

    std::atomic_bool _active_server;
    std::thread _thread;

    void createStatusBuffer();

    TimePoint _deadline;
    std::mutex _mutex;
    std::atomic_bool _send_pending;

    std::future<void> _send_future;
};


}

#endif // BT_ZMQ_PUBLISHER_H
