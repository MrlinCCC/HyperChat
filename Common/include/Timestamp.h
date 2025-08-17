#pragma once
#include <chrono>
#include <string>

class Timestamp
{
public:
    explicit Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    explicit Timestamp(const std::string &timeStr);

    static Timestamp Now();
    std::string ToString() const;

private:
    int64_t m_timestampMicroSeconds;
};
