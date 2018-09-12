#ifndef SIMPLE_BT_NODES_H
#define SIMPLE_BT_NODES_H

#include "behavior_tree_core/behavior_tree.h"

inline BT::NodeStatus SayHello()
{
    std::cout << "Hello!!!" << std::endl;
    return BT::NodeStatus::SUCCESS;
}

inline BT::NodeStatus CheckBattery()
{
    std::cout << "[ Battery: OK ]" << std::endl;
    return BT::NodeStatus::SUCCESS;
}

inline BT::NodeStatus CheckTemperature()
{
    std::cout << "[ Temperature: OK ]" << std::endl;
    return BT::NodeStatus::SUCCESS;
}

//--------------------------------------

class Foo
{
public:
    Foo(): _val(0) {}

    BT::NodeStatus actionOne()
    {
        _val = 42;
        std::cout << "Foo::actionOne -> set val to 42" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }

    BT::NodeStatus actionTwo()
    {
        std::cout << "Foo::actionTwo -> reading val => "<< _val << std::endl;
        _val = 0;
        return BT::NodeStatus::SUCCESS;
    }

private:
     int _val;
};

//--------------------------------------

class CustomAction: public BT::ActionNodeBase
{
public:
    CustomAction(const std::string& name):
        BT::ActionNodeBase(name) {}

    BT::NodeStatus tick() override
    {
        std::cout << "CustomAction: " << this->name() << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
    virtual void halt() override
    {
        setStatus(BT::NodeStatus::IDLE);
    }
};


#endif // SIMPLE_BT_NODES_H
