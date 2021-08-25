//
// Created by DELL on 2021/8/24.
//

#ifndef PERFORMANCEMARKER_TIMESTAMP_H
#define PERFORMANCEMARKER_TIMESTAMP_H

#include <string>

class Timestamp {
public:
    Timestamp()
        : mMicroSecondsSinceEpoch(0)
    {
    }

    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        : mMicroSecondsSinceEpoch(microSecondsSinceEpochArg)
    {
    }

    std::string toFixedSizeString(bool showMicroseconds = true) const;

    static Timestamp now();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t mMicroSecondsSinceEpoch;
};

#endif // PERFORMANCEMARKER_TIMESTAMP_H
