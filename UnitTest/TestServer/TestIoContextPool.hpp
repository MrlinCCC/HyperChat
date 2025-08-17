#include "gtest/gtest.h"
#include "IoContextPool.h"
#include <atomic>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

TEST(TestIoContextPool, AllContextsCanRunTasks)
{
    constexpr int context_count = 4;
    IoContextPool pool(context_count);
    pool.Run();
    std::atomic<int> counter = 0;
    int total_tasks = 20;

    for (int i = 0; i < total_tasks; ++i)
    {
        asio::post(pool.GetIoContext(), [&counter]()
                   {
            std::this_thread::sleep_for(10ms);
            counter.fetch_add(1, std::memory_order_relaxed); });
    }

    std::this_thread::sleep_for(1s);
    EXPECT_EQ(counter.load(), total_tasks);

    pool.Shutdown();
}

TEST(TestIoContextPool, RoundGetIoContext)
{
    constexpr int context_count = 4;
    IoContextPool pool(context_count);
    pool.Run();

    std::vector<asio::io_context *> ioContextRefs;
    for (int i = 0; i < 2 * context_count; ++i)
    {
        ioContextRefs.push_back(&pool.GetIoContext());
    }
    for (int i = 0; i < context_count; ++i)
    {
        EXPECT_EQ(ioContextRefs[i], ioContextRefs[i + context_count]);
    }
    std::set<asio::io_context *> ioContextRefsSet(ioContextRefs.begin(), ioContextRefs.end());
    EXPECT_EQ(ioContextRefsSet.size(), context_count);
}

TEST(TestIoContextPool, ShutdownStopsAllContexts)
{
    constexpr int context_count = 1;
    IoContextPool pool(context_count);
    pool.Run();

    std::atomic<int> counter = 0;
    asio::post(pool.GetIoContext(), [&]()
               {
        std::this_thread::sleep_for(200ms);
        counter.fetch_add(1, std::memory_order_relaxed); });

    pool.Shutdown();

    std::this_thread::sleep_for(300ms);
    EXPECT_LE(counter.load(), 1);
}
