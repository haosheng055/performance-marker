//
// Created by DELL on 2021/6/16.
//
#include "BucketedTimeSeries.h"

template <typename VT, typename CT>
BucketedTimeSeries<VT, CT>::BucketedTimeSeries(
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
bool BucketedTimeSeries<VT, CT>::addValue(const ValueType& value, TimePoint now)
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
bool BucketedTimeSeries<VT, CT>::isEmpty()
{
    // 在构造函数中已经把mFirstTime设置为较大值，如果有数据
    // 插入，那么mFirstTime不会是较大值
    return mFirstTime > mLatestTime;
}

template <typename VT, typename CT>
size_t BucketedTimeSeries<VT, CT>::getBucketIndex(TimePoint now)
{
    // 获取当前时间周期中经历的一段duration
    auto timeInCurrentCycle = now.time_since_epoch() % mDuration;
    return timeInCurrentCycle.count() * mBuckets.size() / mDuration.count();
}

template <typename VT, typename CT>
size_t BucketedTimeSeries<VT, CT>::updateBuckets(TimePoint now)
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
        while (i != newBucketIdx) {
            if (i >= mBuckets.size()) {
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
void BucketedTimeSeries<VT, CT>::getBucketInfo(
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
        + TimePoint(numFullDurations * mDuration);
    *nextBucketStart = Duration(scaledNextBucketStart + mBuckets.size() - 1 / mBuckets.size())
        + TimePoint(numFullDurations * mDuration);
}

template <typename VT, typename CT>
typename CT::time_point BucketedTimeSeries<VT, CT>::getEarliestTime()
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
    earliestTime = std::max(earliestTime, mFirstTime);

    return earliestTime;
}

/*---------------------------------------------------------------------*/

template <typename VT, typename CT>
uint64_t BucketedTimeSeries<VT, CT>::count(
    TimePoint start, TimePoint end) const {
    uint64_t sample_count = 0;
    forEachBucket(
        start,
        end,
        [&](const Bucket& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            sample_count += this->rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mCount);
            return true;
        });

    return sample_count;
}

template <typename VT, typename CT>
VT BucketedTimeSeries<VT, CT>::sum(TimePoint start, TimePoint end) const {
    ValueType total = ValueType();
    forEachBucket(
        start,
        end,
        [&](const Bucket& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            total += this->rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mSum);
            return true;
        });

    return total;
}

template <typename VT, typename CT>
double BucketedTimeSeries<VT,CT>::avg(TimePoint start, TimePoint end) const
{
    uint64_t sample_count = 0;
    ValueType total = ValueType();
    forEachBucket(
        start,
        end,
        [&](const Bucket& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            sample_count += this->template rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mCount);
            total += this->rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mSum);
            return true;
        });
    if(sample_count == 0)
        return 0.0;
    return double(total / sample_count);
}

template <typename VT, typename CT>
ReturnType BucketedTimeSeries<VT,CT>::countRate()
{

}

template <typename VT, typename CT>
template <typename Interval>
Interval BucketedTimeSeries<VT, CT>::elapsed()
{
    if (isEmpty())
        return Duration(0);
    return std::chrono::duration_cast<Interval>(mLatestTime - getEarliestTime() + Duration(1));
}

template <typename VT, typename CT>
template <typename Interval>
Interval BucketedTimeSeries<VT, CT>::elapsed(TimePoint start, TimePoint end)
{
    if (isEmpty())
        return Duration(0);
    start = std::max(start, getEarliestTime());
    end = std::min(end, mLatestTime + Duration(1));
    end = std::max(start, end);
    return std::chrono::duration_cast<Interval>(end - start);
}

template <typename VT, typename CT>
template <typename Function>
void BucketedTimeSeries<VT, CT>::forEachBucket(Function fn) const
{
    typedef typename Duration::rep TimeInt;

    // 做与getBucketInfo类似的操作
    Duration timeMod = mLatestTime.time_since_epoch() % mDuration;
    TimeInt numFullDurations = mLatestTime.time_since_epoch() / mDuration;
    TimeInt scaledTime = timeMod.count() * TimeInt(mBuckets.size());
    TimeInt scaledOffsetInBucket = scaledTime % mDuration.count();
    TimeInt scaledBucketStart = scaledTime - scaledOffsetInBucket;
    TimeInt scaledNextBucketStart = scaledBucketStart + mDuration.count();

    // 开始遍历buckets，从最新的bucket的下一个bucket开始。
    // 由于这个bucket属于上一个time cycle，所以fullDuration的时间
    // 要减去一个duration
    size_t latestBucketIdx = size_t(scaledTime / mBuckets.count());
    size_t idx = latestBucketIdx;
    TimePoint fullDuration = TimePoint(numFullDurations * mDuration) - mDuration;
    TimePoint bucketStart;
    TimePoint nextBucketStart = Duration(
                                    (scaledNextBucketStart + mBuckets.size() - 1) / mBuckets.size())
        + fullDuration;
    while (true) {
        ++idx;
        if (idx >= mBuckets.size()) {
            idx = 0;
            fullDuration += mDuration;
            scaledNextBucketStart = mDuration.count();
        } else {
            scaledNextBucketStart += mDuration.count();
        }

        bucketStart = nextBucketStart;
        nextBucketStart = Duration(
                              (scaledNextBucketStart + mBuckets.size() - 1) / mBuckets.size())
            + fullDuration;

        bool ret = fn(mBuckets[idx], bucketStart, nextBucketStart);
        if (!ret) {
            break;
        }
        if (idx == latestBucketIdx) {
            // all done
            break;
        }
    }
}

template <typename VT, typename CT>
template <typename Function>
void BucketedTimeSeries<VT, CT>::forEachBucket(
    TimePoint start, TimePoint end, Function fn) const
{
    forEachBucket(
        [&start, &end, &fn](
            const Bucket& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            // 如果当前bucket的结束窗口 < 我们指定的start, 说明还没有
            // 遍历到合适的bucketIdx,直接返回true开始下一次遍历
            if (start >= nextBucketStart) {
                return true;
            }
            // 如果当前bucket的开始窗口大于我们指定的end，说明遍历应该结束
            if (end <= bucketStart) {
                return false;
            }
            // 当前bucket的时间窗口与我们指定的[start,end)有重合部分,
            // 调用传入的fn函数
            bool ret = fn(bucket, bucketStart, nextBucketStart);
            return ret;
        });
}

template <typename VT, typename CT>
template <typename ReturnType>
ReturnType BucketedTimeSeries<VT, CT>::rangeAdjust(
    TimePoint bucketStart,
    TimePoint nextBucketStart,
    TimePoint start,
    TimePoint end,
    ReturnType input) const
{
    // 如果nextBucketStart比lastestTime_要大，则需要调整nextBucketStart
    // 为lastestTime_+1，这是因为这个bucket时间的后半部分实际上是没有数据的。
    if (bucketStart <= mLatestTime && nextBucketStart > mLatestTime) {
        nextBucketStart = mLatestTime + Duration(1);
        // 下面种情况意味着这个 bucket 不会包含我们指定时间范围的任何数据。
        if (nextBucketStart <= start) {
            return double();
        }
    }

    if (start <= bucketStart && end >= nextBucketStart) {
        //  这个 bucket 的时间范围是指定时间范围的子集
        return input;
    }

    //  这个 bucket 的时间范围部分包含在指定的时间范围之中
    TimePoint intervalStart = std::max(start, bucketStart);
    TimePoint intervalEnd = std::min(end, nextBucketStart);
    double scale = (intervalEnd - intervalStart) * 1.f / (nextBucketStart - bucketStart);
    return input * scale;
}
