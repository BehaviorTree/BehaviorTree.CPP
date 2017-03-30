#include <HEADER.h>
#include <thread>

BT::CLASSNAME::CONSTRUCTOR(std::string name) : ActionNode::ActionNode(name)
{
    thread_ = std::thread(&ActionTestNode::WaitForTick, this);
}

BT::CLASSNAME::~CONSTRUCTOR() {}

void BT::CLASSNAME::WaitForTick()
{
    while(true)
    {

        // Waiting for the first tick to come
        DEBUG_STDOUT(get_name() << " WAIT FOR TICK");

        tick_engine.Wait();
        DEBUG_STDOUT(get_name() << " TICK RECEIVED");

        // Running state
        set_status(BT::RUNNING);
        // Perform action...

        while(get_status() != BT::HALTED)
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
    set_status(BT::HALTED);
    DEBUG_STDOUT("HALTED state set!");
}


