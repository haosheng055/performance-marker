//
// Created by haosheng on 2021/6/16.
//

#ifndef PERFORMANCE_BUCKETTIMESERIES_INL_H
#define PERFORMANCE_BUCKETTIMESERIES_INL_H

template <typename VT>
BucketedTimeSeries<VT>::BucketedTimeSeries(
    size_t numBuckets, Duration duration)
    : mFirstTime(Duration(1))
    , mLatestTime(Duration(0))
    , mDuration(duration)
{
    if (numBuckets > size_t(mDuration.count())) {
        numBuckets = size_t(mDuration.count());
    }

    mBuckets.resize(numBuckets, BucketType());

    mMutex = std::make_shared<std::mutex>();
}

template <typename VT >
bool BucketedTimeSeries<VT >::addValueAggregated(TimePoint now, const ValueType& total, uint64_t nsamples)
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
    std::lock_guard<std::mutex> guard(*mMutex);
    mTotal.addValueAggregated(total, nsamples);
    mBuckets[bucketIndex].addValueAggregated(total, nsamples);
    return true;
}

template <typename VT >
bool BucketedTimeSeries<VT>::isEmpty() const
{
    // 在构造函数中已经把mFirstTime设置为较大值，如果有数据
    // 插入，那么mFirstTime不会是较大值
    return mFirstTime > mLatestTime;
}

template <typename VT >
size_t BucketedTimeSeries<VT>::update(TimePoint now)
{
    if (isEmpty()) {
        mFirstTime = now;
    }
    if(now > mLatestTime) {
        return updateBuckets(now);
    }
    // 如果传入的时间没有当前有记录的时间新，那么不用管它
    else {
        return getBucketIndex(mLatestTime);
    }
}

template <typename VT >
void BucketedTimeSeries<VT>::clear()
{
    for (BucketType& bucket : mBuckets) {
        bucket.clear();
    }
    mTotal.clear();
    mFirstTime = TimePoint(Duration(1));
    mLatestTime = TimePoint();
}

/*
 * bucket index 的计算策略：
 *
 * buckets_.size() 有可能不能整除 duration_（如duration是28s，size是10），这种情况下
 * 有一些bucket容纳的数据会比其他的多，9个储存2s的buckets，1个储存10s的bucket。我们希望
 * 将数据尽可能均匀地分布在各个bucket之间（而不是使最后一个bucket比所有其他bucket都大得多）
 *
 * buckets的策略是：假设每个bucket都拥有duration_的时间宽度，这样所有的buckets拥有
 * buckets_.size() * duratoin_的时间宽度。
 *
 * 为此，当一个真实的时间戳传递进来时，我们需要先把timestamp乘以buckets_.size()来获得一个
 * 模拟的内部bucket时间戳，然后把这个时间除以duration_就获得了timestamp对应的bucket index
 */
template <typename VT >
size_t BucketedTimeSeries<VT >::getBucketIndex(TimePoint now) const
{
    // 获取当前时间周期中经历的一段duration
    auto timeInCurrentCycle = now.time_since_epoch() % mDuration;
    return timeInCurrentCycle.count() * mBuckets.size() / mDuration.count();
}

template <typename VT >
size_t BucketedTimeSeries<VT >::updateBuckets(TimePoint now)
{
    std::lock_guard<std::mutex> guard(*mMutex);

    size_t currentBucketIdx;
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
    } else if (now >= currentBucketStart + mDuration) {

        for (BucketType& bucket : mBuckets) {
            bucket.clearBucket();
        }
        mTotal.clearBucket();
        return getBucketIndex(now);
    } else {
        size_t newBucket = getBucketIndex(now);
        size_t idx = currentBucketIdx;
        while (idx != newBucket) {
            ++idx;
            if (idx >= mBuckets.size()) {
                idx = 0;
            }
            mTotal -= mBuckets[idx];
            mBuckets[idx].clearBucket();
        }
        return newBucket;
    }
}

/*
 * bucket info 的计算策略：
 *
 * 以duration是28s，size是10为例，这种情况下 duration_不能被 buckets_.size() 整除
 * 我们需要保证每个bucket的时间宽度是一个合适的值。
 * （1）当一个3s的时间戳落进来时，我们先像getBucketIndex()中的那样获取一个内部的scaled的时间：
 * 3*10=30s，然后除以28得到bucket index为1。
 *
 * （2）显然，我们可以得知index为1的 bucket 对应的内部scaled时间范围是[28,28+28)。
 *
 * （3）最后，将内部的scaled时间戳转换为原始的时间戳，需要 除以 buckets_.size()，向上取整还是
 * 向下取整？为了让最后一个 bucket 的时间范围没有损失，我们选择向上取整。
 *
 * 在这里，每五个bucket对应的时间宽度为3、3、3、3、2，每五个一循环。
 */
template <typename VT >
void BucketedTimeSeries<VT >::getBucketInfo(
    TimePoint timePoint, size_t* bucketIdx, TimePoint* bucketStart, TimePoint* nextBucketStart) const
{
    using TimeInt = typename Duration::rep;

    Duration timeMod = timePoint.time_since_epoch() % mDuration;
    TimeInt scaledTime = timeMod.count() * TimeInt(mBuckets.size());
    *bucketIdx = size_t(scaledTime / mDuration.count());

    TimeInt scaledBucketStart = scaledTime - scaledTime % mDuration.count();
    TimeInt scaledNextBucketStart = scaledBucketStart + mDuration.count();
    TimeInt numFullDurations = timePoint.time_since_epoch() / mDuration;
    *bucketStart = Duration((scaledBucketStart + mBuckets.size() - 1) / mBuckets.size())
                   + TimePoint(numFullDurations * mDuration);
    *nextBucketStart = Duration((scaledNextBucketStart + mBuckets.size() - 1) / mBuckets.size())
                       + TimePoint(numFullDurations * mDuration);
}

template <typename VT >
std::chrono::steady_clock::time_point BucketedTimeSeries<VT>::getEarliestTime() const
{
    if (isEmpty() ){
        return TimePoint{};
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

template <typename VT >
uint64_t BucketedTimeSeries<VT >::count(
    TimePoint start, TimePoint end) const
{
    uint64_t sample_count = 0;
    forEachBucket(
        start,
        end,
        [&](const BucketType& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            sample_count += this->rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mCount);
            return true;
        });

    return sample_count;
}

template <typename VT >
VT BucketedTimeSeries<VT >::sum(TimePoint start, TimePoint end) const
{
    ValueType total = ValueType();
    forEachBucket(
        start,
        end,
        [&](const BucketType& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            total += this->rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mSum);
            return true;
        });

    return total;
}

template <typename VT >
double BucketedTimeSeries<VT >::avg(TimePoint start, TimePoint end) const
{
    uint64_t sample_count = 0;
    ValueType total = ValueType();
    forEachBucket(
        start,
        end,
        [&](const BucketType& bucket,
            TimePoint bucketStart,
            TimePoint nextBucketStart) -> bool {
            sample_count += this->template rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mCount);
            total += this->rangeAdjust(
                bucketStart, nextBucketStart, start, end, bucket.mSum);
            return true;
        });
    if (sample_count == 0)
        return 0.0;
    return double(total * 1.0 / sample_count);
}

template <typename VT >
template <typename Interval>
Interval BucketedTimeSeries<VT>::elapsed() const
{
    if (isEmpty())
        return Interval(0);
    return std::chrono::duration_cast<Interval>
        (mLatestTime - getEarliestTime()) + Interval(1);
}

template <typename VT>
template <typename Interval>
Interval BucketedTimeSeries<VT>::elapsed(TimePoint start, TimePoint end) const
{
    if (isEmpty())
        return Interval(0);
    start = std::max(start, getEarliestTime());
    end = std::min(end, mLatestTime + Duration(1));
    end = std::max(start, end);
    return std::chrono::duration_cast<Interval>(end - start);
}

template <typename VT >
template <typename Function>
void BucketedTimeSeries<VT >::forEachBucket(Function fn) const
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

template <typename VT >
template <typename Function>
void BucketedTimeSeries<VT >::forEachBucket(
    TimePoint start, TimePoint end, Function fn) const
{
    forEachBucket(
        [&start, &end, &fn](
            const BucketType& bucket,
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

template <typename VT >
template <typename ReturnType>
ReturnType BucketedTimeSeries<VT >::rangeAdjust(
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


#endif //PERFORMANCE_BUCKETTIMESERIES_INL_H
