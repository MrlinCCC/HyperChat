#include "Timestamp.h"
#include <ctime>
#include <cstdio>

Timestamp::Timestamp() : m_timestampMicroSeconds(0) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : m_timestampMicroSeconds(microSecondsSinceEpoch) {}

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
