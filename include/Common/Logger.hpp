#pragma once

#include <iostream>
#include <string>
#include <Timestamp.hpp>
#include <mutex>
#include <fstream>
#include <sstream>
#include <queue>
#include <condition_variable>
#include <fmt/core.h>
#include <fmt/printf.h>
#include <array>
#include "Semaphore.hpp"
#include "SpinLock.hpp"

enum LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

static std::string LogLevelToString(const LogLevel &level) noexcept
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
        break;
    case LogLevel::INFO:
        return "INFO";
        break;
    case LogLevel::WARN:
        return "WARNING";
        break;
    case LogLevel::ERROR:
        return "ERROR";
        break;
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

    void Log(const std::string &logLine) override
    {
        std::cout << logLine;
        std::cout.flush();
    }
};

class FileWriter : public LogWriter
{
public:
    FileWriter(const std::string &file)
    {
        m_ofs.open(file, std::ios::app);
        if (!m_ofs.is_open())
        {
            throw std::runtime_error("Failed to open log file: " + file);
        }
    }
    void Log(const std::string &logLine) override
    {
        m_ofs << logLine;
        m_ofs.flush();
    }
    ~FileWriter()
    {
        if (m_ofs.is_open())
        {
            m_ofs.close();
        }
    }

private:
    std::ofstream m_ofs;
};

#define CHUNKSIZE (1024 * 256)
#define BUFFERSIZE 64
#define TIMEOUT 1

struct Chunk
{
    Chunk(size_t chunkSize = CHUNKSIZE)
        : m_chunkSize(chunkSize), m_used(0), m_isFull(false)
    {
        m_memory = new char[chunkSize];
    }

    Chunk(const Chunk &other)
        : m_chunkSize(other.m_chunkSize), m_used(other.m_used), m_isFull(other.m_isFull)
    {
        m_memory = new char[m_chunkSize];
        std::memcpy(m_memory, other.m_memory, m_used);
    }

    Chunk &operator=(const Chunk &other)
    {
        if (this != &other)
        {
            delete[] m_memory;

            m_chunkSize = other.m_chunkSize;
            m_used = other.m_used;
            m_isFull = other.m_isFull;

            m_memory = new char[m_chunkSize];
            std::memcpy(m_memory, other.m_memory, m_used);
        }
        return *this;
    }

    void Push(const std::string &logLine)
    {
        size_t length = logLine.size();
        if (m_isFull || m_used + length > m_chunkSize)
        {
            throw std::runtime_error("Chunk push error:chunk space is not enough!");
        }
        std::memcpy(m_memory + m_used, logLine.data(), length);
        m_used += length;
    }
    std::string Extract()
    {
        if (!m_used)
            return "";
        std::string logLines(m_memory, m_used);
        std::memset(m_memory, 0, m_used);
        m_used = 0;
        m_isFull = false;
        return logLines;
    }
    size_t freeSize()
    {
        return m_chunkSize - m_used;
    }

    ~Chunk()
    {
        if (m_memory)
        {
            delete[] m_memory;
            m_memory = nullptr;
        }
    }

    char *m_memory;
    size_t m_chunkSize;
    size_t m_used;
    bool m_isFull;
};

class RingChunkBuffer
{
public:
    RingChunkBuffer(size_t bufferSize = BUFFERSIZE, size_t msTimeout = TIMEOUT)
        : m_bufferSize(bufferSize), m_consumer(0), m_producer(0), m_writeToDisk(0), m_emptyChunk(bufferSize - 1), m_consumeTimeout(msTimeout)
    {
        m_ringChunks = std::vector<Chunk>(bufferSize);
    }

    void Produce(const std::string &logLine)
    {
        m_spLock.lock();
        if (m_ringChunks[m_producer].freeSize() < logLine.size())
        {
            m_writeToDisk.release();
            m_emptyChunk.acquire();
            m_producer = (m_producer + 1) % m_bufferSize;
        }
        m_ringChunks[m_producer].Push(logLine);
        m_spLock.unlock();
    }

    std::string Consume()
    {
        bool acquire = m_writeToDisk.acquire_for(m_consumeTimeout);
        std::string logLines;
        if (acquire)
        {
            logLines = m_ringChunks[m_consumer].Extract();
            m_consumer = (m_consumer + 1) % m_bufferSize;
            m_emptyChunk.release();
        }
        else
        {
            m_spLock.lock();
            if (m_consumer == m_producer)
            {
                Chunk &chunkBuffer = m_ringChunks[m_consumer];

                if (chunkBuffer.m_used > 0)
                {
                    logLines = chunkBuffer.Extract();
                }
            }
            m_spLock.unlock();
        }
        return logLines;
    }

    std::vector<std::string> Flush()
    {
        std::vector<std::string> bufferLogs;
        bufferLogs.reserve(m_bufferSize);
        std::lock_guard<std::mutex> lock(m_mtx);
        while (m_consumer != m_producer)
        {
            bufferLogs.push_back(m_ringChunks[m_consumer].Extract());
            m_consumer = (m_consumer + 1) % m_bufferSize;
        }
        if (m_ringChunks[m_consumer].m_used > 0)
        {
            bufferLogs.push_back(m_ringChunks[m_consumer].Extract());
        }
        m_writeToDisk.reset(0);
        m_emptyChunk.reset(m_bufferSize - 1);
        return bufferLogs;
    }

private:
    size_t m_bufferSize;
    std::vector<Chunk> m_ringChunks;
    size_t m_consumer;
    size_t m_producer;
    std::mutex m_mtx;
    SpinLock m_spLock;
    Semaphore m_writeToDisk;
    Semaphore m_emptyChunk;
    std::chrono::seconds m_consumeTimeout;
};

class Logger
{
public:
    static Logger &getInstance()
    {
        static Logger logger;
        return logger;
    }
    void SetLogLevel(LogLevel level)
    {
        m_level = level;
    }
    void SetConsoleWriter()
    {
        m_writer = std::make_shared<ConsoleWriter>();
    }
    void SetFileWriter(const std::string &file)
    {
        m_writer = std::make_shared<FileWriter>(file);
    }

    void WriteMessage(const std::string &message, LogLevel level)
    {
        if (!m_isRunning || level < m_level)
            return;
        m_ringChunkBuffer.Produce(message);
    }

    void LogFlush()
    {
        auto bufferLogs = m_ringChunkBuffer.Flush();
        for (const std::string &bufferLog : bufferLogs)
        {
            m_writer->Log(bufferLog);
        }
    }

private:
    Logger() : m_level(INFO), m_isRunning(true)
    {
        SetConsoleWriter();
        m_logThread = std::thread(&Logger::ProcessLog, this);
    };
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    void ProcessLog()
    {
        while (m_isRunning)
        {
            std::string logLines = m_ringChunkBuffer.Consume();
            if (logLines.size() > 0)
                m_writer->Log(logLines);
        }
        LogFlush();
    }
    ~Logger()
    {
        m_isRunning = false;
        if (m_logThread.joinable())
        {
            m_logThread.join();
        }
    }

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
#define LOG_ERROR(fmt, ...) Logger::getInstance().WriteMessage(FORMAT_LOGLINE(FORMAT_LOGMESSAGE(fmt, ##__VA_ARGS__), ERROR), ERROR)
#define SET_Console() Logger::getInstance().SetConsoleWriter()
#define SET_LOGFILE(file) Logger::getInstance().SetFileWriter(file)
#define SET_LOGLEVEL(level) Logger::getInstance().SetLogLevel(level)
#define LOG_FLUSH() Logger::getInstance().LogFlush()
