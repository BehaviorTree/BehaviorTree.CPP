#include "behavior_tree_logger/bt_file_logger.h"
#include "behavior_tree_logger/bt_flatbuffer_helper.h"

namespace BT{

FileLogger::FileLogger(BT::TreeNode *root_node, const char *filename, uint16_t buffer_size):
    StatusChangeLogger(root_node),
    _buffer_max_size(buffer_size)
{
    if( _buffer_max_size != 0)
    {
        _buffer.reserve( _buffer_max_size );
    }

    enableTransitionToIdle( true );

    flatbuffers::FlatBufferBuilder builder(1024);
    CreateFlatbuffersBehaviorTree( builder, root_node);

    //-------------------------------------

    _file_os.open( filename, std::ofstream::binary | std::ofstream::out);

    // serialize the length of the buffer in the first 4 bytes
    char size_buff[4];
    flatbuffers::WriteScalar(size_buff, static_cast<int32_t>( builder.GetSize()) );

    _file_os.write( size_buff, 4 );
    _file_os.write( reinterpret_cast<const char*>(builder.GetBufferPointer()),
                    builder.GetSize() );

}

FileLogger::~FileLogger()
{
    this->flush();
    _file_os.close();
}

void FileLogger::callback(TimePoint timestamp, const TreeNode &node, NodeStatus prev_status, NodeStatus status)
{
    std::array<uint8_t,12> buffer = SerializeTransition(
                node.UID(),
                timestamp,
                prev_status,
                status );

    if( _buffer_max_size == 0 )
    {
        _file_os.write( reinterpret_cast<const char*>(buffer.data()), buffer.size() );
    }
    else{
        _buffer.push_back( buffer );
        if( _buffer.size() >= _buffer_max_size)
        {
            this->flush();
        }
    }
}

void FileLogger::flush()
{
    for (const auto& array: _buffer )
    {
        _file_os.write( reinterpret_cast<const char*>(array.data()), array.size() );
    }
    _file_os.flush();
    _buffer.clear();
}



}
