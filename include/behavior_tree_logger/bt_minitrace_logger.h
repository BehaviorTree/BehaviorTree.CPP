#ifndef BT_MINITRACE_LOGGER_H
#define BT_MINITRACE_LOGGER_H

#include <cstring>
#include "abstract_logger.h"
#include "minitrace/minitrace.h"

namespace BT{


class MinitraceLogger: public StatusChangeLogger {

public:
    MinitraceLogger(TreeNode* root_node, const char* filename_json):
        StatusChangeLogger(root_node)
    {
        static bool first_instance = true;
        if( first_instance )
        {
            first_instance = false;
        }
        else{
            throw std::logic_error("Only one instance of MinitraceLogger shall be created");
        }
        minitrace::mtr_register_sigint_handler();
        minitrace::mtr_init(filename_json);
        this->enableTransitionToIdle(true);
    }

    virtual ~MinitraceLogger()
    {
        minitrace::mtr_flush();
        minitrace::mtr_shutdown();
    }

    virtual void callback(TimePoint timestamp, const TreeNode& node,
                          NodeStatus prev_status,
                          NodeStatus status) override
    {
        using namespace minitrace;

        const bool statusCompleted =  (status == NodeStatus::SUCCESS ||
                                       status == NodeStatus::FAILURE);

        const char* category = toStr(node.type());
        const char* name = node.name().c_str();

        if( prev_status == NodeStatus::IDLE && statusCompleted)
        {
            MTR_INSTANT(category, name);
        }
        else if( status == NodeStatus::RUNNING )
        {
            MTR_BEGIN(category, name);
        }
        else if( prev_status == NodeStatus::RUNNING && statusCompleted )
        {
            MTR_END( category, name );
        }
    }

    virtual void flush() override {
        minitrace::mtr_flush();
    }
private:
    TimePoint _prev_time;
};


} // end namespace


#endif // BT_MINITRACE_LOGGER_H
