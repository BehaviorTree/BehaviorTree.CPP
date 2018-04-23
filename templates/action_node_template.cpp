#include <HEADER.h>
#include <thread>

BT::CLASSNAME::CONSTRUCTOR(std::string name) : ActionNode::ActionNode(name)
{
    thread_ = std::thread(&ActionTestNode::WaitForTick, this);
}

BT::CLASSNAME::~CONSTRUCTOR()
{
}

void BT::CLASSNAME::WaitForTick()
{
    while (true)
    {
        // Waiting for the first tick to come
        DEBUG_STDOUT(name() << " WAIT FOR TICK");

        tick_engine.Wait();
        DEBUG_STDOUT(name() << " TICK RECEIVED");

        // Running state
        SetStatus(BT::RUNNING);
        // Perform action...

        while (Status() != BT::HALTED)
        {
            /*HERE THE CODE TO EXECUTE FOR THE ACTION.
	 wHEN THE ACTION HAS FINISHED CORRECLTY, CALL set_status(BT::SUCCESS)
	IF THE ACTION FAILS, CALL set_status(BT::FAILURE)*/
        }
    }
}

void BT::CLASSNAME::Halt()
{
    /*HERE THE CODE TO PERFORM WHEN THE ACTION IS HALTED*/
    SetStatus(BT::HALTED);
    DEBUG_STDOUT("HALTED state set!");
}
