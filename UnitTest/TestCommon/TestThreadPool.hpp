#pragma once
#include <ThreadPool.h>
#include "gtest/gtest.h"

template <typename T>
static T add(T a, T b)
{
    return a + b;
}

void throw_exception()
{
    throw std::runtime_error("Test exception");
}

TEST(TestThreadPool, SingleTaskTest)
{
    ThreadPool pool(4);
    std::future<int> res1 = pool.SubmitTask(add<int>, 1, 2);
    std::future<float> res2 = pool.SubmitTask(add<float>, 1.1f, 2.2f);
    EXPECT_EQ(res1.get(), 3);
    EXPECT_FLOAT_EQ(res2.get(), 3.3f);
}

TEST(TestThreadPool, MultipleTasksTest)
{
    ThreadPool pool(4);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 100; ++i)
    {
        futures.push_back(pool.SubmitTask(add<int>, i, i));
    }

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(futures[i].get(), i + i);
    }
}

TEST(TestThreadPool, ExceptionTest)
{
    ThreadPool pool(1);
    std::future<void> res3 = pool.SubmitTask(throw_exception);
    try
    {
        res3.get();
        assert(false);
    }
    catch (const std::runtime_error &e)
    {
        assert(std::string(e.what()) == "Test exception");
    }
    std::future<int> res1 = pool.SubmitTask(add<int>, 1, 2);
    EXPECT_EQ(res1.get(), 3);
}

TEST(TestThreadPool, ShutdownAndDestructorTest)
{
    {
        std::atomic<int> counter{0};
        ThreadPool pool(2);
        for (int i = 0; i < 10; ++i)
        {
            pool.SubmitTask([&counter]()
                            { ++counter; });
        }
        pool.Shutdown();
        EXPECT_EQ(counter.load(), 10);
    }

    {
        std::atomic<int> counter{0};
        {
            ThreadPool pool(2);
            for (int i = 0; i < 10; ++i)
            {
                pool.SubmitTask([&counter]()
                                { ++counter; });
            }
        }
        EXPECT_EQ(counter.load(), 10);
    }
}
