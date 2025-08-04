#include "ChatService.h"

ChatService::ChatService()
{
    InitDatabase();
}

ChatService &ChatService::GetInstance()
{
    static ChatService service;
    return service;
}

ProtocolResponse::Ptr ChatService::ExecuteService(const std::shared_ptr<Connection> &connection, const ProtocolRequest::Ptr &ProtocolRequest)
{
    std::string type = ProtocolRequest->m_method;

    if (m_serviceHandlerMap.find(type) != m_serviceHandlerMap.end())
    {
        LOG_WARN("Handler for this MethodType not found!");
        // todo router to method no found service
        return nullptr;
    }
    return m_serviceHandlerMap[type](connection, ProtocolRequest);
}

void ChatService::InitDatabase()
{
    std::filesystem::path logFile = std::filesystem::path(PROJECT_LOG_ROOT) / "db/chat.db";
    try
    {
        p_db = std::make_shared<SQLite::Database>(logFile.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        CreateTables();
        LOG_INFO("InitDatabase Success!");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Database init exception: {}", e.what());
    }
}

void ChatService::CreateTables()
{
    try
    {
        // User
        const std::string createUserTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password TEXT NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";
        p_db->exec(createUserTable);
        LOG_INFO("Table user has created!");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Occur error when creating user Table", e.what());
    }
}