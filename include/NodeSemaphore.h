#ifndef NODESEMAPHORE_H
#define NODESEMAPHORE_H

#include <boost/thread.hpp>

class NodeSemaphore
{
private:
    int Value;
    boost::mutex Mutex;
    boost::condition_variable ConditionVariable;
public:
    NodeSemaphore(int InitialValue);
    ~NodeSemaphore();
	void Wait();
	void Signal();
};

#endif
