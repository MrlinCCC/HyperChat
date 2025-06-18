#include "Logger.hpp"
#include <iostream>
#include <filesystem>

#define EPOCH 1000000
#define MULEPOCH 333333

static void InitializeLogger()
{
    Logger &logger = Logger::getInstance();
    logger.SetLogLevel(INFO);
    std::filesystem::path logFile =
        std::filesystem::path(PROJECT_LOG_ROOT) / "BM_LogBenchmark_thread.log";
    if (std::filesystem::exists(logFile))
        std::filesystem::remove(logFile);
    logger.SetFileWriter(logFile.string());
}

void pressureOneThreadTest()
{
    std::string msg(40, '*');
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < EPOCH; ++i)
    {
        LOG_INFO("log info %d %s", i, msg.c_str());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> result = end - start;

    double res = static_cast<double>(result.count());
    printf("OneThread costTime: %f,  %0.0f log/s\n", res, EPOCH / res);
}

void pressureMulThreadTest()
{
    std::string msg(40, '*');
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < MULEPOCH; ++i)
    {
        LOG_INFO("log info %d %s", i, msg.c_str());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> result = end - start;

    double res = static_cast<double>(result.count());
    printf("costTime: %f,  %0.0f log/s\n", res, EPOCH / res);
}

int main()
{
    InitializeLogger();

#if 1
    // 单线程的测试
    pressureOneThreadTest();

#endif

#if 1
    // 多线程的测试
    std::thread th1(&pressureMulThreadTest);
    std::thread th2(&pressureMulThreadTest);
    std::thread th3(&pressureMulThreadTest);

    th1.join();
    th2.join();
    th3.join();

#endif

    return 0;
}