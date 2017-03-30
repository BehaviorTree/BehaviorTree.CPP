#include<behavior_tree.h>



void Execute(BT::ControlNode* root,int TickPeriod_milliseconds)
{
    std::cout << "Start Drawing!" << std::endl;
    // Starts in another thread the drawing of the BT
    std::thread t(&drawTree ,root);

    root->ResetColorState();

    while(true)
    {
           DEBUG_STDOUT( "Ticking the root node !");

        // Ticking the root node
        root->Tick();
        // Printing its state
       //  root->GetNodeState();

        if(root->get_status() != BT::RUNNING  )
        {
            //when the root returns a status it resets the colors of the tree
            root->ResetColorState();
        }
          std::this_thread::sleep_for(std::chrono::milliseconds(TickPeriod_milliseconds));
    }

}
