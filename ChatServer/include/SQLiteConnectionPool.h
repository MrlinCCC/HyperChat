#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <queue>
#include <mutex>
#include "UncopybleAndUnmovable.h"

class SQLiteConnectionPool : public UncopybleAndUnmovable
{
public:
    SQLiteConnectionPool(std::string dbPath, size_t poolSize);

    SQLiteConnectionPool(std::vector<std::shared_ptr<SQLite::Database>>);

    std::shared_ptr<SQLite::Database> GetConnection();

    void RetConnection(std::shared_ptr<SQLite::Database> connection);

private:
    std::string m_dbPath;
    size_t m_poolSize;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::queue<std::shared_ptr<SQLite::Database>> m_pool;
};