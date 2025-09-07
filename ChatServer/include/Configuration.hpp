#pragma once
#include <filesystem>

constexpr size_t CONNECTION_SIZE = 1024;
constexpr size_t RW_THREAD_NUM = 2;
constexpr size_t WORK_THREAD_NUM = 4;
constexpr size_t SQL_CONNECTION_NUMS = 4;
constexpr long long SERVER_SESSION_TIMEOUT_MS = 30000;
constexpr long long SESSION_ALIVE_CHECK_INTERNAL_MS = 10000;
const std::string DB_PATH = (std::filesystem::path(PROJECT_ROOT) / "db/chat.db").string();