//
// Created by DELL on 2021/8/24.
//
#include "log/TimeStamp.h"
#include <chrono>

using namespace std;

Timestamp Timestamp::now()
{
    auto duration = chrono::system_clock::now().time_since_epoch();
    return Timestamp(chrono::duration_cast<chrono::microseconds>(duration).count());
}

std::string Timestamp::toFixedSizeString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(mMicroSecondsSinceEpoch / kMicroSecondsPerSecond);
    struct tm tm_time;
    localtime_s(&tm_time, &seconds);

    if (showMicroseconds)
    {
        int microseconds = static_cast<int>(mMicroSecondsSinceEpoch % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}
