#ifndef BT_ZMQ_PUBLISHER_H
#define BT_ZMQ_PUBLISHER_H

#include <array>
#include <zmq.hpp>
#include "abstract_logger.h"
#include "BT_logger_generated.h"

namespace BT{

class PublisherZMQ: public StatusChangeLogger
{
public:
    PublisherZMQ(TreeNode* root_node, int max_msg_per_second = 100);

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
    std::chrono::milliseconds _min_time_between_msgs;
    std::chrono::high_resolution_clock::time_point _prev_time;

    zmq::context_t _zmq_context;
    zmq::socket_t  _zmq_publisher;
    zmq::socket_t  _zmq_server;

    std::atomic_bool _active_server;
    std::thread _thread;

    void createStatusBuffer();
};


}

#endif // BT_ZMQ_PUBLISHER_H
