#include "Logger.h"

void ConsoleWriter::Log(const std::string &logLine)
{
    std::cout << logLine;
    std::cout.flush();
}

FileWriter::FileWriter(const std::string &file)
{
    m_ofs.open(file, std::ios::app);
    if (!m_ofs.is_open())
    {
        throw std::runtime_error("Failed to open log file: " + file);
    }
}

void FileWriter::Log(const std::string &logLine)
{
    m_ofs << logLine;
    m_ofs.flush();
}

FileWriter::~FileWriter()
{
    if (m_ofs.is_open())
    {
        m_ofs.close();
    }
}

Chunk::Chunk(size_t chunkSize)
    : m_chunkSize(chunkSize), m_used(0), m_isFull(false)
{
    m_memory = new char[chunkSize];
}

Chunk::Chunk(const Chunk &other)
    : m_chunkSize(other.m_chunkSize), m_used(other.m_used), m_isFull(other.m_isFull)
{
    m_memory = new char[m_chunkSize];
    std::memcpy(m_memory, other.m_memory, m_used);
}

Chunk &Chunk::operator=(const Chunk &other)
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

void Chunk::Push(const std::string &logLine)
{
    size_t length = logLine.size();
    if (m_isFull || m_used + length > m_chunkSize)
    {
        throw std::runtime_error("Chunk push error:chunk space is not enough!");
    }
    std::memcpy(m_memory + m_used, logLine.data(), length);
    m_used += length;
}

std::string Chunk::Extract()
{
    if (!m_used)
        return "";
    std::string logLines(m_memory, m_used);
    std::memset(m_memory, 0, m_used);
    m_used = 0;
    m_isFull = false;
    return logLines;
}

size_t Chunk::freeSize()
{
    return m_chunkSize - m_used;
}

Chunk::~Chunk()
{
    if (m_memory)
    {
        delete[] m_memory;
        m_memory = nullptr;
    }
}

RingChunkBuffer::RingChunkBuffer(size_t bufferSize, size_t msTimeout)
    : m_bufferSize(bufferSize), m_consumer(0), m_producer(0),
      m_writeToDisk(0), m_emptyChunk(bufferSize - 1),
      m_consumeTimeout(msTimeout)
{
    m_ringChunks = std::vector<Chunk>(bufferSize);
}

void RingChunkBuffer::Produce(const std::string &logLine)
{
    m_spLock.lock();
    if (m_ringChunks[m_producer].freeSize() < logLine.size())
    {
        m_writeToDisk.Release();
        m_emptyChunk.Acquire();
        m_producer = (m_producer + 1) % m_bufferSize;
    }
    m_ringChunks[m_producer].Push(logLine);
    m_spLock.unlock();
}

std::string RingChunkBuffer::Consume()
{
    bool acquire = m_writeToDisk.Acquire_for(m_consumeTimeout);
    std::string logLines;
    if (acquire)
    {
        logLines = m_ringChunks[m_consumer].Extract();
        m_consumer = (m_consumer + 1) % m_bufferSize;
        m_emptyChunk.Release();
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

std::vector<std::string> RingChunkBuffer::Flush()
{
    std::vector<std::string> bufferLogs;
    bufferLogs.reserve(m_bufferSize);
    std::lock_guard<std::mutex> lock(m_mtx);
    while (m_consumer != m_producer)
    {
        m_writeToDisk.Acquire();
        bufferLogs.push_back(m_ringChunks[m_consumer].Extract());
        m_consumer = (m_consumer + 1) % m_bufferSize;
        m_emptyChunk.Release();
    }
    if (m_ringChunks[m_consumer].m_used > 0)
    {
        bufferLogs.push_back(m_ringChunks[m_consumer].Extract());
    }
    return bufferLogs;
}

Logger::Logger() : m_level(INFO), m_isRunning(true)
{
    SetConsoleWriter();
    m_logThread = std::thread(&Logger::ProcessLog, this);
};

void Logger::ProcessLog()
{
    while (m_isRunning)
    {
        std::string logLines = m_ringChunkBuffer.Consume();
        if (logLines.size() > 0)
            m_writer->Log(logLines);
    }
    LogFlush();
}

Logger::~Logger()
{
    m_isRunning = false;
    if (m_logThread.joinable())
    {
        m_logThread.join();
    }
}

void Logger::SetLogLevel(LogLevel level)
{
    m_level = level;
}

void Logger::SetConsoleWriter()
{
    m_writer = std::make_shared<ConsoleWriter>();
}

void Logger::SetFileWriter(const std::string &file)
{
    m_writer = std::make_shared<FileWriter>(file);
}

void Logger::WriteMessage(const std::string &message, LogLevel level)
{
    if (!m_isRunning || level < m_level)
        return;
    m_ringChunkBuffer.Produce(message);
}

void Logger::LogFlush()
{
    auto bufferLogs = m_ringChunkBuffer.Flush();
    for (const std::string &bufferLog : bufferLogs)
    {
        m_writer->Log(bufferLog);
    }
}
