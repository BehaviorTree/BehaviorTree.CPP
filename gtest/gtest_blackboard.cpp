/* Copyright (C) 2018 Davide Faconti - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/blackboard/blackboard_local.h"

using namespace BT;

//class InitTestNode: public SyncActionNode
//{
//  public:
//    InitTestNode(bool try_to_access_bb, const std::string& name):
//      SyncActionNode(name, {}),
//      _value(0)
//    {
//        if( try_to_access_bb )
//        {
//            // this should throw
//            setOutput(KEY(), 33);
//        }
//    }

//    void onInit() {
//        blackboard()->get(KEY(), _value);
//    }

//    NodeStatus tick()
//    {
//        _value *= 2;
//        setOutput(KEY(), _value);
//        return NodeStatus::SUCCESS;
//    }

//    static const char* KEY() { return "my_entry"; }

//  private:
//    int _value;
//};




/****************TESTS START HERE***************************/

//TEST(BlackboardTest, CheckOInit)
//{
//    auto bb = Blackboard::create<BlackboardLocal>();
//    const auto KEY = InitTestNode::KEY();

//    EXPECT_THROW( InitTestNode(true,"init_test"), std::logic_error );

//    InitTestNode node(false,"init_test");
//    node.setBlackboard(bb);

//    bb->set(KEY, 11 );

//    // this should read and write "my_entry", respectively in onInit() and tick()
//    node.executeTick();

//    ASSERT_EQ( bb->get<int>(KEY), 22 );

//    // check that onInit is executed only once
//    bb->set(KEY, 1 );

//    // since this value is read in OnInit(), the node will not notice the change in bb
//    node.setStatus( NodeStatus::IDLE );
//    node.executeTick();
//    ASSERT_EQ( bb->get<int>(KEY), 44 );
//}
