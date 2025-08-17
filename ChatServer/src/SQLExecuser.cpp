#include "SQLExecuser.h"
#include "filesystem"
#include <functional>
#include "Logger.h"

thread_local std::shared_ptr<SQLite::Database> SQLExecuser::m_currentTransactionConn = nullptr;

SQLExecuser::SQLExecuser(std::shared_ptr<SQLiteConnectionPool> sqlPool) : p_sqlPool(sqlPool)
{
}

std::shared_ptr<SQLite::Database> SQLExecuser::BeginTransaction()
{
    if (m_currentTransactionConn)
        throw std::runtime_error("Transaction already in progress on this thread");
    auto conn = p_sqlPool->GetConnection();
    conn->exec("BEGIN TRANSACTION");
    m_currentTransactionConn = conn;
    return conn;
}

void SQLExecuser::CommitTransaction()
{
    if (!m_currentTransactionConn)
        throw std::runtime_error("No transaction in progress");
    m_currentTransactionConn->exec("COMMIT");
    p_sqlPool->RetConnection(m_currentTransactionConn);
    m_currentTransactionConn.reset();
}

void SQLExecuser::RollbackTransaction()
{
    if (!m_currentTransactionConn)
        throw std::runtime_error("No transaction in progress");
    m_currentTransactionConn->exec("ROLLBACK");
    p_sqlPool->RetConnection(m_currentTransactionConn);
    m_currentTransactionConn.reset();
}
