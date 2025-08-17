
#pragma once
#include "SQLiteCpp/SQLiteCpp.h"
#include "SQLiteConnectionPool.h"
#include "UncopybleAndUnmovable.h"
#include <unordered_map>
#include "functional"

template <typename T, typename = void>
struct HasFromMap : std::false_type
{
};

template <typename T>
struct HasFromMap<T, std::void_t<
	decltype(T::FromMap(std::declval<const std::unordered_map<std::string, std::string>&>()))>>
	: std::bool_constant<std::is_same_v<decltype(T::FromMap(std::declval<const std::unordered_map<std::string, std::string>&>())),
	T>>
{
};

template <typename T>
inline constexpr bool HasFromMapV = HasFromMap<T>::value;

template<typename T>
void BindValue(SQLite::Statement& stmt, int index, T&& value) {
	stmt.bind(index, std::forward<T>(value));
}

inline void BindValue(SQLite::Statement& stmt, int index, std::nullptr_t) {
	stmt.bind(index);
}

class SQLExecuser : public UncopybleAndUnmovable
{
public:
	explicit SQLExecuser(std::shared_ptr<SQLiteConnectionPool> sqlPool);

	template <typename T, typename... Args>
	std::enable_if_t<HasFromMapV<T>, std::vector<T>> ExecuteQuery(const std::string& query, Args &&...args)
	{

		auto connection = GetConnectionForExecute();
		SQLite::Statement stmt(*connection, query);

		if constexpr (sizeof...(args) > 0)
		{
			int idx = 1;
			BindValue(stmt, idx++, std::forward<Args>(args));
		}

		std::vector<T> result;
		while (stmt.executeStep())
		{
			std::unordered_map<std::string, std::string> row;
			for (int i = 0; i < stmt.getColumnCount(); ++i)
			{
				row[stmt.getColumnName(i)] = stmt.getColumn(i).getText();
			}
			result.push_back(T::FromMap(row));
		}

		ReleaseConnectionIfNotInTransaction(connection);
		return result;
	}

	template <typename... Args>
	std::vector<std::unordered_map<std::string, std::string>> ExecuteQuery(const std::string& query, Args &&...args)
	{

		auto connection = GetConnectionForExecute();
		SQLite::Statement stmt(*connection, query);

		if constexpr (sizeof...(args) > 0)
		{
			int idx = 1;
			BindValue(stmt, idx++, std::forward<Args>(args));
		}

		std::vector<std::unordered_map<std::string, std::string>> result;
		while (stmt.executeStep())
		{
			std::unordered_map<std::string, std::string> row;
			for (int i = 0; i < stmt.getColumnCount(); ++i)
			{
				row[stmt.getColumnName(i)] = stmt.getColumn(i).getText();
			}
			results.push_back(std::move(row));
		}

		ReleaseConnectionIfNotInTransaction(connection);
		return result;
	}

	template <typename... Args>
	int32_t ExecuteUpdate(const std::string& query, Args &&...args)
	{
		auto connection = GetConnectionForExecute();
		SQLite::Statement stmt(*connection, query);
		if constexpr (sizeof...(args) > 0)
		{
			int idx = 1;
			(stmt.bind(idx++, std::forward<Args>(args)), ...);
		}
		int rowsAffected = stmt.exec();
		ReleaseConnectionIfNotInTransaction(connection);
		return rowsAffected;
	}

	template <typename T, typename... Args>
	std::enable_if_t<HasFromMapV<T>, T> ExecuteUpdateAndReturn(const std::string& query, Args &&...args)
	{

		auto connection = GetConnectionForExecute();
		std::string modifiedQuery = query;
		if (modifiedQuery.find("RETURNING") == std::string::npos)
		{
			modifiedQuery += " RETURNING *";
		}

		SQLite::Statement stmt(*connection, query);

		if constexpr (sizeof...(args) > 0)
		{
			int idx = 1;
			(stmt.bind(idx++, std::forward<Args>(args)), ...);
		}

		if (!stmt.executeStep())
		{
			p_sqlPool->ReleaseConnectionIfNotInTransaction(connection);
			throw std::runtime_error("Insert failed or no row returned.");
		}

		std::vector<T> result;
		std::unordered_map<std::string, std::string> row;
		for (int i = 0; i < stmt.getColumnCount(); ++i)
		{
			row[stmt.getColumnName(i)] = stmt.getColumn(i).getText();
		}

		RetConnection(connection);
		return T::FromMap(row);
	}

	std::shared_ptr<SQLite::Database> BeginTransaction();

	void CommitTransaction();

	void RollbackTransaction();

	inline const std::shared_ptr<SQLite::Database>& GetCurrentTransactionConn() const
	{
		return m_currentTransactionConn;
	}

private:
	std::shared_ptr<SQLite::Database> GetConnectionForExecute()
	{
		if (m_currentTransactionConn)
			return m_currentTransactionConn;
		return p_sqlPool->GetConnection();
	}

	void ReleaseConnectionIfNotInTransaction(std::shared_ptr<SQLite::Database> conn)
	{
		if (conn != m_currentTransactionConn)
			p_sqlPool->RetConnection(conn);
	}

	std::shared_ptr<SQLiteConnectionPool> p_sqlPool;

	thread_local static std::shared_ptr<SQLite::Database> m_currentTransactionConn;
};