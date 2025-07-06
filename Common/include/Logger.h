#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <fstream>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <chrono>
#include <fmt/core.h>
#include <fmt/printf.h>
#include "Timestamp.h"
#include "Semaphore.h"
#include "SpinLock.h"

enum LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERR
};

static std::string LogLevelToString(const LogLevel &level) noexcept
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARN:
        return "WARNING";
    case LogLevel::ERR:
        return "ERR";
    default:
        return "UNDEFINE";
    }
}

template <typename... Args>
std::string FormatLogMessage(const std::string &fmt_str, Args &&...args)
{
    try
    {
        if (fmt_str.find('%') != std::string::npos)
        {
            return fmt::sprintf(fmt_str.c_str(), std::forward<Args>(args)...);
        }
        else
        {
            return fmt::format(fmt_str, std::forward<Args>(args)...);
        }
    }
    catch (const fmt::format_error &e)
    {
        throw std::invalid_argument(std::string("Log formatting failed: ") + e.what());
    }
}

class LogWriter
{
public:
    using ptr = std::shared_ptr<LogWriter>;
    LogWriter() = default;
    virtual ~LogWriter() noexcept {}
    virtual void Log(const std::string &logLine) = 0;
};

class ConsoleWriter : public LogWriter
{
public:
    ConsoleWriter() = default;
    ~ConsoleWriter() = default;
    void Log(const std::string &logLine) override;
};

class FileWriter : public LogWriter
{
public:
    explicit FileWriter(const std::string &file);
    ~FileWriter();
    void Log(const std::string &logLine) override;

private:
    std::ofstream m_ofs;
};

#define CHUNKSIZE (1024 * 256)
#define BUFFERSIZE 64
#define TIMEOUT 1

struct Chunk
{
    explicit Chunk(size_t chunkSize = CHUNKSIZE);
    Chunk(const Chunk &other);
    Chunk &operator=(const Chunk &other);
    ~Chunk();

    void Push(const std::string &logLine);
    std::string Extract();
    size_t freeSize();

    char *m_memory;
    size_t m_chunkSize;
    size_t m_used;
    bool m_isFull;
};

class RingChunkBuffer
{
public:
    explicit RingChunkBuffer(size_t bufferSize = BUFFERSIZE, size_t msTimeout = TIMEOUT);

    void Produce(const std::string &logLine);
    std::string Consume();
    std::vector<std::string> Flush();

private:
    size_t m_bufferSize;
    std::vector<Chunk> m_ringChunks;
    size_t m_consumer;
    size_t m_producer;
    std::mutex m_mtx;
    SpinLock m_spLock;
    CountSemaphore m_writeToDisk;
    CountSemaphore m_emptyChunk;
    std::chrono::seconds m_consumeTimeout;
};

class Logger
{
public:
    static Logger &getInstance();

    void SetLogLevel(LogLevel level);
    void SetConsoleWriter();
    void SetFileWriter(const std::string &file);

    void WriteMessage(const std::string &message, LogLevel level);
    void LogFlush();

private:
    Logger();
    ~Logger();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    void ProcessLog();

    LogLevel m_level;
    LogWriter::ptr m_writer;
    std::thread m_logThread;
    RingChunkBuffer m_ringChunkBuffer;
    std::atomic<bool> m_isRunning;
};

#define FORMAT_LOGMESSAGE(fmt, ...) FormatLogMessage(fmt, ##__VA_ARGS__)
#define FORMAT_LOGLINE(fmtted, level) \
    fmt::format("[{}] [{}] [{}:{}]:{}\n", Timestamp::Now().ToString(), LogLevelToString(level), __FUNCTION__, __LINE__, fmtted)
#define LOG_BASE(msg, level) Logger::getInstance().WriteMessage(msg, level)
#define LOG_DEBUG(fmt, ...) Logger::getInstance().WriteMessage(FORMAT_LOGLINE(FORMAT_LOGMESSAGE(fmt, ##__VA_ARGS__), DEBUG), DEBUG)
#define LOG_INFO(fmt, ...) Logger::getInstance().WriteMessage(FORMAT_LOGLINE(FORMAT_LOGMESSAGE(fmt, ##__VA_ARGS__), INFO), INFO)
#define LOG_WARN(fmt, ...) Logger::getInstance().WriteMessage(FORMAT_LOGLINE(FORMAT_LOGMESSAGE(fmt, ##__VA_ARGS__), WARN), WARN)
#define LOG_ERROR(fmt, ...) Logger::getInstance().WriteMessage(FORMAT_LOGLINE(FORMAT_LOGMESSAGE(fmt, ##__VA_ARGS__), ERR), ERR)
#define SET_Console() Logger::getInstance().SetConsoleWriter()
#define SET_LOGFILE(file) Logger::getInstance().SetFileWriter(file)
#define SET_LOGLEVEL(level) Logger::getInstance().SetLogLevel(level)
#define LOG_FLUSH() Logger::getInstance().LogFlush()