#include "SQLiteConnectionPool.h"

SQLiteConnectionPool::SQLiteConnectionPool(std::string dbPath, size_t poolSize) : m_dbPath(dbPath), m_poolSize(poolSize)
{
    for (int i = 0; i < poolSize; ++i)
    {
        m_pool.push(std::make_shared<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));
    }
}

SQLiteConnectionPool::SQLiteConnectionPool(std::vector<std::shared_ptr<SQLite::Database>> connections) : m_poolSize(connections.size())
{
    for (auto &conn : connections)
    {
        m_pool.push(conn);
    }
}

std::shared_ptr<SQLite::Database> SQLiteConnectionPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]()
              { return !m_pool.empty(); });
    auto conn = m_pool.front();
    m_pool.pop();
    return conn;
}

void SQLiteConnectionPool::RetConnection(std::shared_ptr<SQLite::Database> connection)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_pool.push(connection);
    m_cv.notify_one();
}