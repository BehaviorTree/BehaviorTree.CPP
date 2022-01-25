#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/basic_types.h"

#include <string>
#include <array>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// The mocked version of the base.
struct MockedAsyncActionNode : public BT::AsyncActionNode
{
    using BT::AsyncActionNode::AsyncActionNode;
    MOCK_METHOD0(tick, BT::NodeStatus());

    // Tick while the node is running.
    BT::NodeStatus spinUntilDone()
    {
        do
        {
            executeTick();
        } while (status() == BT::NodeStatus::RUNNING);
        return status();
    }

    // Expose the setStatus call.
    using BT::AsyncActionNode::setStatus;
};

// The fixture taking care of the node-setup.
struct MockedAsyncActionFixture : public testing::Test
{
    BT::NodeConfiguration config;
    MockedAsyncActionNode sn;
    MockedAsyncActionFixture() : sn("node", config)
    {
    }
};

// Parameters for the terminal node states.
struct NodeStatusFixture : public testing::WithParamInterface<BT::NodeStatus>,
                           public MockedAsyncActionFixture
{
};

INSTANTIATE_TEST_CASE_P(/**/, NodeStatusFixture,
                        testing::Values(BT::NodeStatus::SUCCESS, BT::NodeStatus::FAILURE));

TEST_P(NodeStatusFixture, normal_routine)
{
    // Test verifies the "normal" operation: We correctly propagate the result
    // from the tick to the caller.
    const BT::NodeStatus state = GetParam();

    // Setup the mock-expectations.
    EXPECT_CALL(sn, tick()).WillOnce(testing::Invoke([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return state;
    }));

    // Spin the node and check the final status.
    ASSERT_EQ(sn.spinUntilDone(), state);
}

TEST_F(MockedAsyncActionFixture, no_halt)
{
    // Test verifies that halt returns immediately, if the node is idle. It
    // further checks if the halt-flag is resetted correctly.
    sn.halt();
    ASSERT_TRUE(sn.isHaltRequested());

    // Below we further verify that the halt flag is cleaned up properly.
    const BT::NodeStatus state{BT::NodeStatus::SUCCESS};
    EXPECT_CALL(sn, tick()).WillOnce(testing::Return(state));

    // Spin the node and check.
    ASSERT_EQ(sn.spinUntilDone(), state);
    ASSERT_FALSE(sn.isHaltRequested());
}

TEST_F(MockedAsyncActionFixture, halt)
{
    // Test verifies that calling halt() is blocking.
    bool release = false;
    std::mutex m;
    std::condition_variable cv;

    const BT::NodeStatus state{BT::NodeStatus::SUCCESS};
    EXPECT_CALL(sn, tick()).WillOnce(testing::Invoke([&]() {
        // Sleep until we send the release signal.
        std::unique_lock<std::mutex> l(m);
        while (!release)
            cv.wait(l);

        return state;
    }));

    // Start the execution.
    sn.executeTick();

    // Try to halt the node (cv will block it...)
    std::future<void> halted = std::async(std::launch::async, [&]() { sn.halt(); });
    ASSERT_EQ(halted.wait_for(std::chrono::milliseconds(10)), std::future_status::timeout);
    ASSERT_EQ(sn.status(), BT::NodeStatus::RUNNING);

    // Release the method.
    {
        std::unique_lock<std::mutex> l(m);
        release = true;
        cv.notify_one();
    }

    // Wait for the future to return.
    halted.wait();
    ASSERT_EQ(sn.status(), state);
}

TEST_F(MockedAsyncActionFixture, exception)
{
    // Verifies that we can recover from the exceptions in the tick method:
    // 1) catch the exception, 2) re-raise it in the caller thread.

    // Setup the mock.
    EXPECT_CALL(sn, tick()).WillOnce(testing::Invoke([&]() {
        throw std::runtime_error("This is not good!");
        return BT::NodeStatus::SUCCESS;
    }));

    ASSERT_ANY_THROW(sn.spinUntilDone());

    // Now verify that the exception is cleared up (we succeed).
    sn.setStatus(BT::NodeStatus::IDLE);
    testing::Mock::VerifyAndClearExpectations(&sn);
    const BT::NodeStatus state{BT::NodeStatus::SUCCESS};
    EXPECT_CALL(sn, tick()).WillOnce(testing::Return(state));
    ASSERT_EQ(sn.spinUntilDone(), state);
}
