/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/16
 *
 */

#ifndef PERFORMANCE_BUCKETEDTIMESERIES_H
#define PERFORMANCE_BUCKETEDTIMESERIES_H

#include <chrono>
#include <vector>
#include "Bucket.h"

template <typename VT, typename CT = std::chrono::steady_clock>
class BucketedTimeSeires {
public:
    using ValueType = VT;
    using Clock = CT;
    using Duration = typename Clock::duration;
    using TimePoint = typename Clock::time_point;
    using Bucket = Bucket<ValueType>;

    BucketedTimeSeires(size_t numBuckets, Duration duration);

    bool addValue(const ValueType& value, TimePoint now);

    uint64_t count() { return mTotal.mCount; }

    ValueType sum() { return mTotal.mSum; }

    double avg() { return mTotal.avg(); }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate()
    {
        return ReturnType(mTotal.mCount / elapsed<Interval>().count());
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType valueRate()
    {
        return ReturnType(mTotal.mSum / elapsed<Interval>().count());
    }

//    size_t update(TimePoint now);

    /* 获取指定时间落入的 bucket 下标 */
    size_t getBucketIndex(TimePoint now);

    /* 清除bucktes数组中过时的数据 */
    size_t updateBuckets(TimePoint now);

    /* 获取时间戳now 所对应的bucket的时间范围 */
    void getBucketInfo(TimePoint timePoint,size_t* bucketIdx,TimePoint* bucketStart,
        TimePoint* nextBucketStart);

    TimePoint getEarliestTime();

    template <typename Interval = std::chrono::seconds>
    Interval elapsed();

    bool isEmpty();

    TimePoint getFirstTime() const { return mFirstTime;}
    TimePoint getLatestTime() const { return mLatestTime;}
    Duration getDuration() const { return mDuration;}

private:
    TimePoint mFirstTime;
    TimePoint mLatestTime;
    Duration mDuration;
    Bucket mTotal; //一个记录所有数据的bucket
    std::vector<Bucket> mBuckets;
};

#endif //PERFORMANCE_BUCKETEDTIMESERIES_H
