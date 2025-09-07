#pragma once

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <atomic>
#include "Timer.h"

class TimerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        timer = std::make_unique<Timer>(100, 100);
    }

    void TearDown() override
    {
        if (timer)
        {
            timer->Shutdown();
        }
    }

    std::unique_ptr<Timer> timer;
    std::atomic<int> callbackCount{0};
};

TEST_F(TimerTest, RunAfterBasic)
{
    timer->Start();

    auto start = std::chrono::steady_clock::now();
    std::atomic<bool> callbackCalled{false};

    timer->RunAfter(200, [&callbackCalled, start]()
                    {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        EXPECT_GE(duration, 200);
        EXPECT_LE(duration, 300);
        callbackCalled = true; });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(callbackCalled);
}

TEST_F(TimerTest, RunEveryRepeating)
{
    timer->Start();
    std::atomic<int> executionCount{0};
    auto start = std::chrono::steady_clock::now();

    timer->RunEvery(100, [&executionCount, start]()
                    { executionCount++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_GE(executionCount, 4);
    EXPECT_LE(executionCount, 5);
}

TEST_F(TimerTest, MultipleEveryTimers)
{
    timer->Start();

    std::atomic<int> count1{0};
    std::atomic<int> count2{0};

    timer->RunEvery(150, [&count1]()
                    { count1++; });

    timer->RunEvery(200, [&count2]()
                    { count2++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(650));

    EXPECT_GE(count1, 4);
    EXPECT_LE(count1, 5);
    EXPECT_GE(count2, 3);
    EXPECT_LE(count2, 4);
}

TEST_F(TimerTest, ManyTimers)
{
    timer->Start();

    const int numTimers = 50;
    std::atomic<int> completedCount{0};

    for (int i = 0; i < numTimers; ++i)
    {
        timer->RunAfter(100 + i * 50, [&completedCount, i]()
                        { completedCount++; });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100 + numTimers * 50 + 100));
    EXPECT_EQ(completedCount, numTimers);
}

TEST_F(TimerTest, RunAtSpecificTime)
{
    timer->Start();

    std::atomic<bool> called{false};
    auto targetTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);

    timer->RunAt(targetTime, [&called, targetTime]()
                 {
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - targetTime).count();
        EXPECT_NEAR(static_cast<int>(diff), 0, 100);
        called = true; });

    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    EXPECT_TRUE(called);
}

TEST_F(TimerTest, WheelExpansion)
{

    auto smallTimer = std::make_unique<Timer>(100, 3);

    smallTimer->Start();
    std::atomic<int> longDelayCalled{false};

    smallTimer->RunAfter(400, [&longDelayCalled]()
                         { longDelayCalled = true; });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_TRUE(longDelayCalled);

    std::this_thread::sleep_for(std::chrono::milliseconds(123));

    std::atomic<int> longerDelayCalled{false};
    smallTimer->RunAfter(1000, [&longerDelayCalled]()
                         { longerDelayCalled = true; });

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    EXPECT_TRUE(longerDelayCalled);
    ;
    smallTimer->Shutdown();
}

TEST_F(TimerTest, CancelByShutdown)
{
    timer->Start();

    std::atomic<bool> shouldNotBeCalled{false};

    timer->RunAfter(500, [&shouldNotBeCalled]()
                    { shouldNotBeCalled = true; });

    timer->Shutdown();

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    EXPECT_FALSE(shouldNotBeCalled);
}

TEST_F(TimerTest, ImmediateExpiration)
{
    timer->Start();

    std::atomic<bool> immediateCalled{false};
    std::atomic<bool> pastCalled{false};

    timer->RunAt(std::chrono::steady_clock::now(), [&immediateCalled]()
                 { immediateCalled = true; });

    timer->RunAt(std::chrono::steady_clock::now() - std::chrono::seconds(1),
                 [&pastCalled]()
                 {
                     pastCalled = true;
                 });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(immediateCalled);
    EXPECT_TRUE(pastCalled);
}
