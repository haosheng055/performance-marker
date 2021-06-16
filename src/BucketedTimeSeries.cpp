//
// Created by DELL on 2021/6/16.
//
#include "BucketedTimeSeries.h"

template <typename VT, typename CT>
BucketedTimeSeires<VT, CT>::BucketedTimeSeires(
    size_t numBuckets, Duration duration)
    : mFirstTime(Duration(1))
    , mLatestTime(Duration())
    , mDuration(duration)
{
    if (numBuckets > size_t(mDuration.count())) {
        numBuckets = size_t(mDuration.count());
    }

    mBuckets.resize(numBuckets, Bucket());
}

template <typename VT, typename CT>
bool BucketedTimeSeires<VT, CT>::addValue(const ValueType& value, TimePoint now)
{
    int bucketIndex;
    if (isEmpty()) {
        // 记录到的第一个数据
        mFirstTime = now;
        mLatestTime = now;
        bucketIndex = getBucketIndex(now);
    } else if (now == mLatestTime) {
        // now还在当前时间内，无需额外操作
        bucketIndex = getBucketIndex(now);
    } else if (now > mLatestTime) {
        // now是一个稍新的时间
        bucketIndex = updateBuckets(now);
    } else {
        // now是一个稍早一些的过去的时间, 需要check 这个时间是否还在
        // 我们的跟踪的时间范围内
        if (now < getEarliestTime())
            return false;
        bucketIndex = getBucketIndex(now);
    }
    mTotal.addValue(value, 1);
    mBuckets[bucketIndex].addValue(value, 1);
    return true;
}

template <typename VT, typename CT>
bool BucketedTimeSeires<VT, CT>::isEmpty()
{
    // 在构造函数中已经把mFirstTime设置为较大值，如果有数据
    // 插入，那么mFirstTime不会是较大值
    return mFirstTime > mLatestTime;
}

template <typename VT, typename CT>
size_t BucketedTimeSeires<VT, CT>::getBucketIndex(TimePoint now)
{
    // 获取当前时间周期中经历的一段duration
    auto timeInCurrentCycle = now.time_since_epoch() % mDuration;
    return timeInCurrentCycle.count() * mBuckets.size() / mDuration.count();
}

template <typename VT, typename CT>
size_t BucketedTimeSeires<VT, CT>::updateBuckets(TimePoint now)
{
    size_t currentBucketIdx = getBucketIndex(now);
    TimePoint currentBucketStart;
    TimePoint nextBucketStart;
    // 先计算出自从上次插入的数据落入的bucket的信息
    getBucketInfo(mLatestTime, &currentBucketIdx, &currentBucketStart, &nextBucketStart);

    mLatestTime = now;
    // 现在的timestamp有三种情况，（1）时间过去没多久，now依然落在上次update的bucket上
    // （2）时间过去了多于duration_，buckets中的所有数据都过时了，对所有的bucket进行clear()
    // （3）时间过去了duration_之内，buckets中的部分数据过时，对这部分数据进行clear()
    if (now < nextBucketStart) {
        return currentBucketIdx;
    } else if (now > nextBucketStart + mDuration) {
        for (Bucket& bucket : mBuckets) {
            bucket.clearBucket();
        }
        mTotal.clearBucket();
        return getBucketIndex(now);
    } else {
        size_t newBucketIdx = getBucketIndex(now);
        size_t i = currentBucketIdx + 1;
        while(i != newBucketIdx) {
            if(i >=mBuckets.size()){
                i = 0;
            }
            mTotal -= mBuckets[i];
            mBuckets[i].clearBucket();
            i++;
        }
        return newBucketIdx;
    }
}

template <typename VT, typename CT>
void BucketedTimeSeires<VT,CT>::getBucketInfo(
    TimePoint timePoint, size_t* bucketIdx, TimePoint* bucketStart, TimePoint* nextBucketStart)
{
    using TimeInt = typename Duration::rep;

    Duration timeMod = timePoint.time_since_epoch() % mDuration;
    TimeInt scaledTime = timeMod.count() * TimeInt(mBuckets.size());
    *bucketIdx = size_t(scaledTime / mDuration.count());

    TimeInt scaledBucketStart = scaledTime - scaledTime % mDuration.count();
    TimeInt scaledNextBucketStart = scaledTime + mDuration.count();
    TimeInt numFullDurations = timePoint.time_since_epoch() / mDuration;
    *bucketStart = Duration(scaledBucketStart + mBuckets.size() - 1 / mBuckets.size())
        + TimePoint(numFullDurations*mDuration);
    *nextBucketStart = Duration(scaledNextBucketStart + mBuckets.size() - 1 / mBuckets.size())
                   + TimePoint(numFullDurations*mDuration);
}

template <typename VT, typename CT>
typename CT::time_point BucketedTimeSeires<VT,CT>::getEarliestTime()
{
    if (isEmpty()()) {
        return TimePoint();
    }

    TimePoint earliestTime;
    size_t currentBucket;
    TimePoint currentBucketStart;
    TimePoint nextBucketStart;
    getBucketInfo(
        mLatestTime, &currentBucket, &currentBucketStart, &nextBucketStart);
    earliestTime = nextBucketStart - mDuration;
    earliestTime = std::max(earliestTime,mFirstTime);

    return earliestTime;
}

/*---------------------------------------------------*/


template <typename VT, typename CT>
template <typename Interval>
Interval BucketedTimeSeires<VT,CT>::elapsed()
{
    if(isEmpty())
        return Duration(0);
    return std::chrono::duration_cast<Interval>
        (mLatestTime - getEarliestTime() + Duration(1));
}

