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
        if (level < m_level)
            return;
        LogEvent localEvent = std::forward<T>(event);
        localEvent.m_level = level;

        std::lock_guard<std::mutex> lock(m_mtx);
        m_logQueue.push(std::move(localEvent));
        m_cv.notify_one();
    }

private:
    Logger() : m_level(INFO), m_isRunning(true)
    {
        m_logThread = std::thread(&Logger::ProcessLog, this);
        SetConsoleWriter();
    };
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    void ProcessLog()
    {
        while (!m_logQueue.empty() || m_isRunning)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]
                      { return !m_logQueue.empty() || !m_isRunning; });
            if (!m_logQueue.empty())
            {
                const LogEvent &event = m_logQueue.front();
                m_writer->Log(event);
                m_logQueue.pop();
            }
        }
    }
    ~Logger()
    {
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_isRunning = false;
        }
        m_cv.notify_one();
        m_logThread.join();
    }

    LogLevel m_level;
    LogWriter::ptr m_writer;
    std::thread m_logThread;
    std::mutex m_mtx;
    std::queue<LogEvent> m_logQueue;
    std::condition_variable m_cv;
    bool m_isRunning;
};

#define MAKE_LOG_EVENT(fmt, ...) LogEvent((fmt), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_BASE(fmt, level, ...) Logger::getInstance().WriteLogEvent(MAKE_LOG_EVENT(fmt, ##__VA_ARGS__), level);
#define LOG_DEBUG(fmt, ...) LOG_BASE(fmt, LogLevel::DEBUG, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_BASE(fmt, LogLevel::INFO, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_BASE(fmt, LogLevel::WARN, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_BASE(fmt, LogLevel::ERROR, ##__VA_ARGS__)
