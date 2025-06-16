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
std::string LogMessageFormat(const std::string &fmt_str, Args &&...args)
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

struct LogEvent
{
    template <typename... Args>
    LogEvent(const std::string &message, const std::string fileName, uint32_t line, Args &&...args)
        : m_message(LogMessageFormat(message, std::forward<Args>(args)...)),
          m_level(LogLevel::DEBUG), m_fileName(fileName), m_line(line), m_time(Timestamp::Now().ToString()){};

    LogEvent() = default;

    std::string m_message;
    LogLevel m_level;
    std::string m_fileName;
    uint32_t m_line;
    std::string m_time;
};

static std::string LogEventToString(const LogEvent &event)
{
    std::ostringstream oss;
    oss << "[" << event.m_time << "] "
        << "[" << LogLevelToString(event.m_level) << "] "
        << "[" << event.m_fileName << ":" << event.m_line << "]:"
        << event.m_message;
    return oss.str();
}

class LogWriter
{
public:
    using ptr = std::shared_ptr<LogWriter>;

    LogWriter() = default;

    virtual ~LogWriter() noexcept {}

    virtual void Log(const LogEvent &event) = 0;
};

class ConsoleWriter : public LogWriter
{
public:
    ConsoleWriter() = default;
    ~ConsoleWriter() = default;

    void Log(const LogEvent &event) override
    {
        std::ostream &os = event.m_level >= ERROR ? std::cerr : std::cout;
        os << LogEventToString(event) << std::endl;
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
    void Log(const LogEvent &event) override
    {
        m_ofs << LogEventToString(event) << std::endl;
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

#define CHUNKSIZE 1024
#define BUFFERSIZE 4
#define TIMEOUT 1000

struct Chunk
{
    Chunk(uint32_t chunkSize = CHUNKSIZE)
        : m_chunkSize(chunkSize), m_used(0), m_isFull(false)
    {
        m_memory = std::vector<LogEvent>(chunkSize);
    }

    template <typename T>
    void Push(T &&event)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_isFull)
            throw std::runtime_error("Push error:chunk full!");
        m_memory[m_used] = std::forward<T>(event);
        if (++m_used == m_chunkSize)
            m_isFull = true;
    }
    std::vector<LogEvent> Extract()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (!m_used)
            return {};
        std::vector<LogEvent> events(std::make_move_iterator(m_memory.begin()),
                                     std::make_move_iterator(m_memory.begin() + m_used));

        m_used = 0;
        m_isFull = false;
        return events;
    }

    std::vector<LogEvent> m_memory;
    uint32_t m_chunkSize;
    uint32_t m_used;
    bool m_isFull;
    std::mutex m_mtx;
};

class RingChunkBuffer
{
public:
    RingChunkBuffer(uint32_t bufferSize = BUFFERSIZE, uint32_t msTimeout = TIMEOUT)
        : m_bufferSize(bufferSize), m_consumer(0), m_producer(0), m_consumeTimeout(msTimeout)
    {
        m_ringChunks = std::vector<Chunk>(bufferSize);
        m_consumeTimeout = std::chrono::microseconds(msTimeout);
    }

    template <typename T>
    void Produce(T &&event)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cvWaitEmptyChunk.wait(lock, [this]()
                                { return !m_ringChunks[m_producer].m_isFull ||
                                         ((m_producer + 1) % m_bufferSize != m_consumer); });
        if (m_ringChunks[m_producer].m_isFull)
        {
            m_producer = (m_producer + 1) % m_bufferSize;
            m_cvWriteToDisk.notify_one();
        }
        m_ringChunks[m_producer].Push(event);
    }
    std::vector<LogEvent> Consume()
    {
        bool acquire = false;
        std::vector<LogEvent> logEvents = {};

        std::unique_lock<std::mutex> lock(m_mtx);
        acquire = m_cvWriteToDisk.wait_for(lock, m_consumeTimeout, [this]()
                                           { return m_consumer != m_producer; });
        if (acquire)
        {
            lock.unlock();
            Chunk &chunkBuffer = m_ringChunks[m_consumer];
            logEvents = chunkBuffer.Extract();
            m_consumer = (m_consumer + 1) % m_bufferSize;
            m_cvWaitEmptyChunk.notify_all();
        }
        else
        {
            Chunk &chunkBuffer = m_ringChunks[m_consumer];
            if (chunkBuffer.m_used > 0)
            {
                logEvents = chunkBuffer.Extract();
            }
        }
        return logEvents;
    }

    void Flush(LogWriter::ptr writer)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        while (m_consumer != m_producer)
        {
            std::vector<LogEvent> LogEvents = m_ringChunks[m_consumer].Extract();
            for (LogEvent &event : LogEvents)
            {
                writer->Log(event);
            }
            m_consumer = (m_consumer + 1) % m_bufferSize;
        }
        if (m_ringChunks[m_consumer].m_used > 0)
        {
            std::vector<LogEvent> LogEvents = m_ringChunks[m_consumer].Extract();
            for (LogEvent &event : LogEvents)
            {
                writer->Log(event);
            }
        }
    }

private:
    uint32_t m_bufferSize;
    std::vector<Chunk> m_ringChunks;
    uint32_t m_consumer;
    uint32_t m_producer;
    std::mutex m_mtx;
    std::condition_variable m_cvWriteToDisk;
    std::condition_variable m_cvWaitEmptyChunk;
    std::chrono::microseconds m_consumeTimeout;
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
    template <typename T>
    void WriteLogEvent(T &&event, LogLevel level)
    {
        if (!m_isRunning || level < m_level)
            return;
        LogEvent localEvent = std::forward<T>(event);
        localEvent.m_level = level;
        m_ringChunkBuffer.Produce(std::move(localEvent));
    }

    void LogFlush()
    {
        m_ringChunkBuffer.Flush(m_writer);
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
            std::vector<LogEvent> logEvents = m_ringChunkBuffer.Consume();
            for (LogEvent &event : logEvents)
            {
                m_writer->Log(event);
            }
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

#define MAKE_LOG_EVENT(fmt, ...) LogEvent((fmt), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_BASE(fmt, level, ...) Logger::getInstance().WriteLogEvent(MAKE_LOG_EVENT(fmt, ##__VA_ARGS__), level);
#define LOG_DEBUG(fmt, ...) LOG_BASE(fmt, LogLevel::DEBUG, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_BASE(fmt, LogLevel::INFO, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_BASE(fmt, LogLevel::WARN, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_BASE(fmt, LogLevel::ERROR, ##__VA_ARGS__)
