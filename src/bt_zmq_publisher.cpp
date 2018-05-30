#include "behavior_tree_logger/bt_zmq_publisher.h"
#include "behavior_tree_logger/bt_flatbuffer_helper.h"

namespace BT {

void PublisherZMQ::createStatusBuffer()
{
    _status_buffer.clear();
    recursiveVisitor(_root_node, [this](TreeNode* node)
    {
        size_t index = _status_buffer.size();
        _status_buffer.resize( index + 3 );
        flatbuffers::WriteScalar<uint16_t>( &_status_buffer[index],   node->UID() );
        flatbuffers::WriteScalar<int8_t>(   &_status_buffer[index+2],
                static_cast<int8_t>( convertToFlatbuffers(node->status())) );
    });
}

PublisherZMQ::PublisherZMQ(TreeNode *root_node,
                           int max_msg_per_second):
    StatusChangeLogger(root_node),
    _root_node(root_node),
    _min_time_between_msgs( std::chrono::milliseconds(1000) / max_msg_per_second ),
    _zmq_context(1),
    _zmq_publisher( _zmq_context, ZMQ_PUB ),
    _zmq_server( _zmq_context, ZMQ_REP )
{
    static bool first_instance = true;
    if( first_instance )
    {
        first_instance = false;
    }
    else{
        throw std::logic_error("Only one instance of PublisherZMQ shall be created");
    }

    flatbuffers::FlatBufferBuilder builder(1024);
    CreateFlatbuffersBehaviorTree( builder, root_node);

    _tree_buffer.resize(builder.GetSize());
    memcpy( _tree_buffer.data(), builder.GetBufferPointer(), builder.GetSize() );

    _zmq_publisher.bind( "tcp://*:1666" );
    _zmq_server.bind( "tcp://*:1667" );

    _active_server = true;

    _thread = std::thread([this]()
    {
        while(_active_server)
        {
            zmq::message_t req;
            try{
                _zmq_server.recv( &req );
                zmq::message_t reply ( _tree_buffer.size() );
                memcpy( reply.data(), _tree_buffer.data(), _tree_buffer.size() );
                _zmq_server.send( reply );
            }
            catch( zmq::error_t& err)
            {
                _active_server = false;
            }
        }
    });

    createStatusBuffer();
}

PublisherZMQ::~PublisherZMQ()
{
    _active_server = false;
    if( _thread.joinable())
    {
        _thread.join();
    }
    flush();
}

void PublisherZMQ::callback(TimePoint timestamp, const TreeNode &node, NodeStatus prev_status, NodeStatus status)
{
    using namespace std::chrono;

    std::array<uint8_t,12> transition =  SerializeTransition(node.UID(),
                                                             timestamp, prev_status, status);

    _transition_buffer.push_back( transition );

    if( (timestamp - _prev_time) >= _min_time_between_msgs )
    {
        _prev_time = timestamp;
        flush();
    };
}

void PublisherZMQ::flush()
{
    const size_t msg_size = _status_buffer.size() + 8 +
            (_transition_buffer.size() * 12);

    zmq::message_t message ( msg_size );
    uint8_t* data_ptr = static_cast<uint8_t*>( message.data() );

    // first 4 bytes are the side of the header
    flatbuffers::WriteScalar<uint32_t>( data_ptr, _status_buffer.size() );
    data_ptr += sizeof(uint32_t);
    // copy the header part
    memcpy( data_ptr, _status_buffer.data(), _status_buffer.size() );
    data_ptr += _status_buffer.size();

    // first 4 bytes are the side of the transition buffer
    flatbuffers::WriteScalar<uint32_t>( data_ptr, _transition_buffer.size() );
    data_ptr += sizeof(uint32_t);

    for (auto& transition: _transition_buffer)
    {
        memcpy( data_ptr, transition.data(), transition.size() );
        data_ptr += transition.size();
    }

    _zmq_publisher.send(message);
    _transition_buffer.clear();

    // rebuild _status_buffer for the next time
    createStatusBuffer();
}

}
