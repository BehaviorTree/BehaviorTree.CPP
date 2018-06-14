#ifndef BT_COUT_LOGGER_H
#define BT_COUT_LOGGER_H

#include <cstring>
#include "abstract_logger.h"


namespace BT{



/**
 * @brief AddStdCoutLoggerToTree. Give  the root node of a tree,
 * a simple callback is subscribed to any status change of each node.
 *
 *
 * @param root_node
 * @return Important: the returned shared_ptr must not go out of scope,
 *         otherwise the logger is removed.
 */

class StdCoutLogger: public StatusChangeLogger {

public:

    StdCoutLogger(TreeNode* root_node):
        StatusChangeLogger(root_node)
    {
        static bool first_instance = true;
        if( first_instance )
        {
            first_instance = false;
        }
        else{
            throw std::logic_error("Only one instance of StdCoutLogger shall be created");
        }
    }

    virtual void callback(TimePoint timestamp,
                          const TreeNode& node,
                          NodeStatus prev_status,
                          NodeStatus status) override
    {
        using namespace std::chrono;

        constexpr const char* whitespaces = "                         ";
        constexpr const size_t ws_count = 25;

        double since_epoch =  duration<double>( timestamp.time_since_epoch() ).count();
        printf("[%.3f]: %s%s %s -> %s\n",
               since_epoch,
               node.name().c_str(),
               &whitespaces[ std::min( ws_count, node.name().size()) ],
                toStr(prev_status, true),
                toStr(status, true) );
    }

    virtual void flush() override { std::cout << std::flush; }
};


} // end namespace


#endif // BT_COUT_LOGGER_H
