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
    {}

    virtual ~StdCoutLogger() = default;

    virtual void callback(const TreeNode& node,
                          NodeStatus prev_status,
                          NodeStatus status)
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();

        constexpr const char* whitespaces = "                    ";
        constexpr const size_t ws_count = 20;

        double since_epoch =  duration<double>( now.time_since_epoch() ).count();
        printf("[%.3f]: %s%s %s -> %s\n",
               since_epoch,
               node.name().c_str(),
               &whitespaces[ std::min( ws_count, node.name().size()) ],
                toStr(prev_status, true),
                toStr(status, true) );
    }
};


} // end namespace


#endif // BT_COUT_LOGGER_H
