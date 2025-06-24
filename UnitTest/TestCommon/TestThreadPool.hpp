#pragma once
#include <ThreadPool.hpp>
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

TEST(TestThreadPool, ResultTest)
{
    ThreadPool pool(4);
    std::future<int> res1 = pool.submitTask(add<int>, 1, 2);
    std::future<float> res2 = pool.submitTask(add<float>, 1.1f, 2.2f);
    EXPECT_EQ(res1.get(), 3);
    EXPECT_FLOAT_EQ(res2.get(), 3.3f);
}

TEST(TestThreadPool, ExceptionTest)
{
    ThreadPool pool(1);
    std::future<void> res3 = pool.submitTask(throw_exception);
    try
    {
        res3.get();
        assert(false);
    }
    catch (const std::runtime_error &e)
    {
        assert(std::string(e.what()) == "Test exception");
    }
    std::future<int> res1 = pool.submitTask(add<int>, 1, 2);
    EXPECT_EQ(res1.get(), 3);
}