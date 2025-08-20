#include "SQLExecuser.h"
#include "filesystem"
#include <functional>
#include "Logger.h"

thread_local std::shared_ptr<SQLite::Database> SQLExecuser::m_currentTransactionConn = nullptr;
thread_local std::stack<std::string> SQLExecuser::m_savepointStack;

SQLExecuser::SQLExecuser(std::shared_ptr<SQLiteConnectionPool> sqlPool) : p_sqlPool(sqlPool)
{
}

std::shared_ptr<SQLite::Database> SQLExecuser::BeginTransaction()
{
	if (m_currentTransactionConn) {
		auto conn = m_currentTransactionConn;
		std::string savepointName = "nested_transaction_" + std::to_string(m_savepointStack.size() + 1);
		conn->exec("SAVEPOINT " + savepointName);
		m_savepointStack.push(savepointName);
		return conn;
	}
	auto conn = p_sqlPool->GetConnection();
	conn->exec("BEGIN TRANSACTION");
	m_currentTransactionConn = conn;
	return conn;
}


void SQLExecuser::CommitTransaction()
{
	if (!m_currentTransactionConn) {
		LOG_WARN("No transaction in progress");
		return;
	}
	if (!m_savepointStack.empty()) {
		std::string savepoint = m_savepointStack.top();
		m_currentTransactionConn->exec("RELEASE SAVEPOINT " + savepoint);
		m_savepointStack.pop();
	}
	else {
		m_currentTransactionConn->exec("COMMIT");
		p_sqlPool->RetConnection(m_currentTransactionConn);
		m_currentTransactionConn.reset();
	}
}

void SQLExecuser::RollbackTransaction()
{
	if (!m_currentTransactionConn) {
		LOG_WARN("No transaction in progress");
		return;
	}
	if (!m_savepointStack.empty()) {
		std::string savepoint = m_savepointStack.top();
		m_currentTransactionConn->exec("ROLLBACK TO SAVEPOINT " + savepoint);
		m_savepointStack.pop();
	}
	else {
		m_currentTransactionConn->exec("ROLLBACK");
		p_sqlPool->RetConnection(m_currentTransactionConn);
		m_currentTransactionConn.reset();
	}
}
