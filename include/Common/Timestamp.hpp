#pragma once
#include <stdint.h>
#include <chrono>
#include <string>
#include <memory>

class Timestamp
{
public:
    explicit Timestamp() : m_timestampMicroSeconds(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpoch) : m_timestampMicroSeconds(microSecondsSinceEpoch) {}

    static Timestamp Now()
    {
        auto now = std::chrono::system_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count();
        return Timestamp(us);
    }

    std::string ToString() const
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

private:
    int64_t m_timestampMicroSeconds;
};