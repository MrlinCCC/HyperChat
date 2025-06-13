#pragma once
#include <Logger.hpp>
#include "gtest/gtest.h"
#include <filesystem>

TEST(TestLog, ConsoleLogTest)
{
    Logger &logger = Logger::getInstance();
    logger.SetConsoleWriter();
    std::streambuf *oldCoutStreamBuf = std::cout.rdbuf();
    std::streambuf *oldCerrStreamBuf = std::cerr.rdbuf();
    std::stringstream ss;
    std::cout.rdbuf(ss.rdbuf());
    std::cerr.rdbuf(ss.rdbuf());
    logger.SetLogLevel(DEBUG);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");
    logger.SetLogLevel(INFO);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");
    logger.SetLogLevel(WARN);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");
    logger.SetLogLevel(ERROR);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout.rdbuf(oldCoutStreamBuf);
    std::cout.rdbuf(oldCerrStreamBuf);

    std::string line;
    std::getline(ss, line);
    EXPECT_NE(line.find(",./"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(DEBUG)), std::string::npos);
    std::getline(ss, line);
    EXPECT_NE(line.find("abc"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(INFO)), std::string::npos);
    std::getline(ss, line);
    EXPECT_NE(line.find("123"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(WARN)), std::string::npos);
    std::getline(ss, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);

    std::getline(ss, line);
    EXPECT_NE(line.find("abc"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(INFO)), std::string::npos);
    std::getline(ss, line);
    EXPECT_NE(line.find("123"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(WARN)), std::string::npos);
    std::getline(ss, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);

    std::getline(ss, line);
    EXPECT_NE(line.find("123"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(WARN)), std::string::npos);
    std::getline(ss, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);

    std::getline(ss, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);
}

TEST(TestLog, FileLogTest)
{
    Logger &logger = Logger::getInstance();
    std::filesystem::path logFile = std::filesystem::path(PROJECT_LOG_ROOT) / "TestLog_FileLogTest.log";
    if (std::filesystem::exists(logFile))
        std::filesystem::remove(logFile);
    logger.SetFileWriter(logFile.string());
    logger.SetLogLevel(DEBUG);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");
    logger.SetLogLevel(INFO);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");
    logger.SetLogLevel(WARN);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");
    logger.SetLogLevel(ERROR);
    LOG_DEBUG(",./");
    LOG_INFO("abc");
    LOG_WARN("123");
    LOG_ERROR("/*-");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::ifstream logFileStream(logFile);
    ASSERT_TRUE(logFileStream.is_open());

    std::string line;
    std::getline(logFileStream, line);
    EXPECT_NE(line.find(",./"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(DEBUG)), std::string::npos);
    std::getline(logFileStream, line);
    EXPECT_NE(line.find("abc"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(INFO)), std::string::npos);
    std::getline(logFileStream, line);
    EXPECT_NE(line.find("123"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(WARN)), std::string::npos);
    std::getline(logFileStream, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);

    std::getline(logFileStream, line);
    EXPECT_NE(line.find("abc"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(INFO)), std::string::npos);
    std::getline(logFileStream, line);
    EXPECT_NE(line.find("123"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(WARN)), std::string::npos);
    std::getline(logFileStream, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);

    std::getline(logFileStream, line);
    EXPECT_NE(line.find("123"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(WARN)), std::string::npos);
    std::getline(logFileStream, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);

    std::getline(logFileStream, line);
    EXPECT_NE(line.find("/*-"), std::string::npos);
    EXPECT_NE(line.find(LogLevelToString(ERROR)), std::string::npos);
}

TEST(TestLog, FormatLogTest)
{
    Logger &logger = Logger::getInstance();
    std::filesystem::path logFile = std::filesystem::path(PROJECT_LOG_ROOT) / "TestLog_FormatLogTest.log";
    if (std::filesystem::exists(logFile))
        std::filesystem::remove(logFile);
    logger.SetFileWriter(logFile.string());
    logger.SetLogLevel(INFO);
    LOG_INFO("Simple integer: %d", 42);
    LOG_INFO("Integer and string: %d %s", 1, "test");
    LOG_INFO("Floating point: %.2f", 3.14159);
    LOG_INFO("Multiple integers: %d %d %d", 10, 20, 30);
    LOG_INFO("Character: %c", 'A');
    LOG_INFO("String with spaces: '%s'", "hello world");
    LOG_INFO("Mix: int=%d, float=%.1f, char=%c, str=%s", 7, 2.718, 'Z', "mix");
    LOG_INFO("Percent sign: %%");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::ifstream logFileStream(logFile);
    ASSERT_TRUE(logFileStream.is_open());

    std::string line;

    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Simple integer: 42"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Integer and string: 1 test"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Floating point: 3.14"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Multiple integers: 10 20 30"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Character: A"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("String with spaces: 'hello world'"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Mix: int=7, float=2.7, char=Z, str=mix"), std::string::npos);
    std::getline(logFileStream, line);
    ASSERT_NE(line.find("Percent sign: %"), std::string::npos);

    EXPECT_THROW(LOG_INFO("Expect two ints: %d %d", 42), std::invalid_argument);
    EXPECT_THROW(LOG_INFO("Expect int but pass string: %d", "oops"), std::invalid_argument);
    EXPECT_THROW(LOG_INFO("Illegal format specifier: %q", 123), std::invalid_argument);

    EXPECT_THROW(LOG_INFO("Expect two ints: {} {}", 42), std::invalid_argument);
    EXPECT_THROW(LOG_INFO("Illegal format specifier: {q}", 123), std::invalid_argument);
}