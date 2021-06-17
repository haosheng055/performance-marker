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
class BucketedTimeSeries {
public:
    using ValueType = VT;
    using Clock = CT;
    using Duration = typename Clock::duration;
    using TimePoint = typename Clock::time_point;
    using Bucket = Bucket<ValueType>;

    BucketedTimeSeries(size_t numBuckets, Duration duration);

    bool addValue(const ValueType& value, TimePoint now);

    uint64_t count() { return mTotal.mCount; }

    /* 计算[start,end)这个前闭后开的时间段的数据个数 */
    uint64_t count(TimePoint start, TimePoint end) const;

    ValueType sum() { return mTotal.mSum; }

    /* 计算[start,end)这个前闭后开的时间段的数据value总和 */
    ValueType sum(TimePoint start, TimePoint end) const;

    double avg() { return mTotal.avg(); }

    /* 计算[start,end)这个前闭后开的时间段的平均值 */
    double avg(TimePoint start, TimePoint end) const;

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate()
    {
        return ReturnType(mTotal.mCount / elapsed<Interval>().count());
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(TimePoint start, TimePoint end)
    {
        uint64_t intervalCount = count(start, end);
        Interval interval = elapsed<Interval>(start, end);
        return ReturnType(intervalCount / interval.count());
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType valueRate()
    {
        return ReturnType(mTotal.mSum / elapsed<Interval>().count());
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType valueRate(TimePoint start, TimePoint end)
    {
        uint64_t intervalSum = sum(start, end);
        Interval interval = elapsed<Interval>(start, end);
        return ReturnType(intervalSum / interval.count());
    }


    /* 获取指定时间落入的 bucket 下标 */
    size_t getBucketIndex(TimePoint now);

    /* 获取时间戳now 所对应的bucket的时间范围 */
    void getBucketInfo(TimePoint timePoint,size_t* bucketIdx,TimePoint* bucketStart,
        TimePoint* nextBucketStart);

    TimePoint getEarliestTime();

    /* 返回一段逝去的时间elapsed。
     *
     * elapsed开始于能跟踪到的最早的数据，结束于最新的数据的时间。也就是说，如果
     * 经历的时间小于整个duration，会返回第一个数据以来的所有时间；如果经历的时间
     * 大于整个duration，我们会丢弃一些数据，会返回能跟踪到的最早的数据以来的时间。
     */
    template <typename Interval = std::chrono::seconds>
    Interval elapsed();

    /* 注意：[start,end)是前闭后开的区间 */
    template <typename Interval = std::chrono::seconds>
    Interval elapsed(TimePoint start,TimePoint end);

    template <typename Function>
    void forEachBucket(Function function) const;

    template <typename Function>
    void forEachBucket(TimePoint start, TimePoint end, Function fn) const;

    template <typename ReturnType>
    ReturnType rangeAdjust(
        TimePoint bucketStart,
        TimePoint nextBucketStart,
        TimePoint start,
        TimePoint end,
        ReturnType input) const;

    bool isEmpty();

    TimePoint getFirstTime() const { return mFirstTime;}
    TimePoint getLatestTime() const { return mLatestTime;}
    Duration getDuration() const { return mDuration;}
    const Bucket& getBucketByIdx(size_t index) const { return mBuckets[index]; }

private:
    /* 清除bucktes数组中过时的数据 */
    size_t updateBuckets(TimePoint now);

    TimePoint mFirstTime;
    TimePoint mLatestTime;
    Duration mDuration;
    Bucket mTotal; //一个记录所有数据的bucket
    std::vector<Bucket> mBuckets;
};

#endif //PERFORMANCE_BUCKETEDTIMESERIES_H
