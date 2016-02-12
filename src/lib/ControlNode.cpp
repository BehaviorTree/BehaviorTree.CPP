#include <ControlNode.h>

using namespace BT;

ControlNode::ControlNode(std::string Name) : TreeNode::TreeNode(Name)
{
    Type = Control;
}

ControlNode::~ControlNode() {}

void ControlNode::AddChild(TreeNode* Child)
{
    // Checking if the Child is not already present
    for (unsigned int i=0; i<ChildNodes.size(); i++)
    {
        if (ChildNodes[i] == Child)
        {
            throw BehaviorTreeException("'" + Child->Name + "' is already a '" + Name + "' child.");
        }
    }

    ChildNodes.push_back(Child);
    ChildStates.push_back(Idle);
}

unsigned int ControlNode::GetChildrenNumber()
{
    return ChildNodes.size();
}

bool ControlNode::Halt()
{
    // Lock acquistion
    boost::lock_guard<boost::mutex> LockGuard(StateMutex);

    State = Halted;
    return true;
}

bool ControlNode::WriteState(NodeState StateToBeSet)
{
    // Lock acquistion
    boost::lock_guard<boost::mutex> LockGuard(StateMutex);

    State = StateToBeSet;
    return true;
}


std::vector<TreeNode*> ControlNode::GetChildren()
{
    return ChildNodes;
}


void ControlNode::ResetColorState()
{

    SetColorState(Idle);
    for(unsigned int i = 0; i < ChildNodes.size(); i++)
    {
        ChildNodes[i]->ResetColorState();
    }
}

void ControlNode::HaltChildren(int i){
    for(unsigned int j=i; j<ChildNodes.size(); j++)
    {
        if (ChildNodes[j]->Type != Action && ChildStates[j] == Running)
        {
            // if the control node was running:
            // halting it;
            ChildNodes[j]->Halt();

            // sync with it (it's waiting on the semaphore);
            ChildNodes[j]->Semaphore.Signal();

            std::cout << Name << " halting child number " << j << "!" << std::endl;
        }
        else if (ChildNodes[j]->Type == Action && ChildNodes[j]->ReadState() == Running)
        {
            std::cout << Name << " trying halting child number " << j << "..." << std::endl;

            // if it's a action node that hasn't finished its job:
            // trying to halt it:
            if (ChildNodes[j]->Halt() == false)
            {
                // this means that, before this node could set its child state
                // to "Halted", the child had already written the action outcome;
                // sync with him ignoring its state;
                ChildNodes[j]->Semaphore.Signal();

                std::cout << Name << " halting of child number " << j << " failed!" << std::endl;
            }

            std::cout << Name << " halting of child number " << j << " succedeed!" << std::endl;
        }
        else if (ChildNodes[j]->Type == Action && ChildNodes[j]->ReadState() != Idle)
        {
            // if it's a action node that has finished its job:
            // ticking it without saving its returning state;
            ChildNodes[j]->Semaphore.Signal();
        }

        // updating its vector cell
        ChildStates[j] = Idle;
    }

 }


int ControlNode::GetDepth()
{
        int depMax = 0;
        int dep = 0;
        for (int i = 0; i < ChildNodes.size(); i++)
        {
            dep = (ChildNodes[i]->GetDepth());
           if (dep > depMax)
           {
               depMax = dep;
           }

        }
      return 1 + depMax;
}

