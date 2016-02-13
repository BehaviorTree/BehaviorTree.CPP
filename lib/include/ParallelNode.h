#ifndef PARALLELNODE_H
#define PARALLELNODE_H

#include <ControlNode.h>

#include <limits>

namespace BT
{
    class ParallelNode : public ControlNode
    {
    private:
        // N threshold
        unsigned int N;

        // Number of returned Success, Failure and Running states
        unsigned int Successes;
        unsigned int Failures;
        unsigned int Runnings;

        // Update states vector
        std::vector<bool> ChildStatesUpdated;

        // State update
        bool StateUpdate;
    public:
        // Constructor
        ParallelNode(std::string Name);
        ~ParallelNode();

        // the method used to set N
        void SetThreshold(unsigned int N);
        // Method that retuns the type
        int GetType();
        // The method that is going to be executed by the thread
        void Exec();
    };
}

#endif
