#ifndef BT_FILE_LOGGER_H
#define BT_FILE_LOGGER_H

#include <fstream>
#include <deque>
#include <array>
#include "abstract_logger.h"

namespace BT{

class FileLogger: public StatusChangeLogger {

public:
    FileLogger(TreeNode* root_node, const char* filename, uint16_t buffer_size);

    virtual ~FileLogger();

    virtual void callback(TimePoint timestamp,
                          const TreeNode& node,
                          NodeStatus prev_status,
                          NodeStatus status) override;

    virtual void flush() override;

private:

    std::ofstream _file_os;

    std::chrono::high_resolution_clock::time_point _start_time;

    std::vector< std::array<uint8_t,12> > _buffer;

    bool _buffer_max_size;
};


} // end namespace


#endif // BT_FILE_LOGGER_H
