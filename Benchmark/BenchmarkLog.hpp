#pragma once
#include <Logger.hpp>
#include <string>
#include <chrono>
#include <filesystem>
#include <benchmark/benchmark.h>
#include <fmt/core.h>
#include <mutex>
#include <condition_variable>

#define EPOCH 100000

static std::once_flag init_flag;
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

static void BM_LogBenchmark(benchmark::State &state)
{
    std::call_once(init_flag, [&]
                   { InitializeLogger(); });
    std::string msg(40, '*');
    for (auto _ : state)
    {
        for (int i = 0; i < EPOCH; ++i)
        {
            LOG_INFO("log info %d %s", i, msg.c_str());
        }
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * EPOCH);
}

BENCHMARK(BM_LogBenchmark)->Threads(1)->Iterations(8);
BENCHMARK(BM_LogBenchmark)->Threads(2)->Iterations(4);
BENCHMARK(BM_LogBenchmark)->Threads(4)->Iterations(2);
BENCHMARK(BM_LogBenchmark)->Threads(8)->Iterations(1);