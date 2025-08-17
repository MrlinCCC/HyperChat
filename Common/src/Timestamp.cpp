#include "Timestamp.h"
#include <ctime>
#include <cstdio>
#include <iomanip>
#include <sstream>

Timestamp::Timestamp() : m_timestampMicroSeconds(Timestamp::Now().m_timestampMicroSeconds) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : m_timestampMicroSeconds(microSecondsSinceEpoch) {}

Timestamp::Timestamp(const std::string &timeStr)
{
    std::tm tm = {};
    std::istringstream ss(timeStr);

    std::string timePart;
    std::string microsecondsPart = "000000";

    size_t pos = timeStr.find('.');
    if (pos != std::string::npos)
    {
        timePart = timeStr.substr(0, pos);
        microsecondsPart = timeStr.substr(pos + 1);
    }
    else
    {
        timePart = timeStr;
    }

    ss.str(timePart);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail())
    {
        throw std::invalid_argument("Failed to parse time string: " + timeStr);
    }

    std::time_t time = std::mktime(&tm);
    if (time == -1)
    {
        throw std::invalid_argument("Failed to convert to time_t");
    }

    int microseconds = std::stoi(microsecondsPart.substr(0, 6));
    m_timestampMicroSeconds = static_cast<int64_t>(time) * 1000000 + microseconds;
}

Timestamp Timestamp::Now()
{
    auto now = std::chrono::system_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  now.time_since_epoch())
                  .count();
    return Timestamp(us);
}

std::string Timestamp::ToString() const
{
    time_t seconds = m_timestampMicroSeconds / 1000000;
    int microseconds = m_timestampMicroSeconds % 1000000;

    struct tm ptm;
    localtime_s(&ptm, &seconds);
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
             "%4d-%02d-%02d %02d:%02d:%02d.%06d",
             ptm.tm_year + 1900,
             ptm.tm_mon + 1,
             ptm.tm_mday,
             ptm.tm_hour,
             ptm.tm_min,
             ptm.tm_sec,
             microseconds);

    return std::string(buffer);
}
