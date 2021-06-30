//
// Created by DELL on 2021/6/16.
//

#ifndef PERFORMANCE_MULTILEVELTIMESERIES_INL_H
#define PERFORMANCE_MULTILEVELTIMESERIES_INL_H

template <typename VT>
MultiLevelTimeSeries<VT>::MultiLevelTimeSeries(
    size_t nBuckets, size_t nLevels, const Duration levelDurations[])
    : mCachedTime(), mCachedSum(0), mCachedCount(0)
{
    mMutex = std::make_shared<std::mutex>();
    mLevels.reserve(nLevels);
    for (size_t i = 0; i < nLevels; ++i) {
        mLevels.emplace_back(nBuckets, levelDurations[i]);
    }
}

template <typename VT >
MultiLevelTimeSeries<VT>::MultiLevelTimeSeries(
    size_t nBuckets, std::initializer_list<Duration> durations)
    : mCachedTime(), mCachedSum(0), mCachedCount(0)
{
    mMutex = std::make_shared<std::mutex>();
    mLevels.reserve(durations.size());
    size_t i = 0;
    for (auto dur : durations) {
        mLevels.emplace_back(nBuckets, dur);
        i++;
    }
}

template <typename VT >
void MultiLevelTimeSeries<VT>::addValue(
    TimePoint now, const ValueType& val)
{
    addValue(now, val, 1);
}

template <typename VT >
void MultiLevelTimeSeries<VT>::addValue(
    TimePoint now, const ValueType& val, uint64_t times)
{
    addValueAggregated(now, val * times, times);
}

template <typename VT >
void MultiLevelTimeSeries<VT>::addValueAggregated(
    TimePoint now, const ValueType& total, uint64_t nsamples)
{
    // 如果已经过了一段时间，需要将缓存中的数据更新到每个level中
    if (mCachedTime != now) {
        flush();
        mCachedTime = now;
    }
    // 将传入的数据写入缓存
    std::lock_guard<std::mutex> guard(*mMutex);
    mCachedSum += total;
    mCachedCount += nsamples;
}

template <typename VT>
void MultiLevelTimeSeries<VT>::update(TimePoint now)
{
    flush();
    for (size_t i = 0; i < mLevels.size(); ++i) {
        mLevels[i].update(now);
    }
}

template <typename VT >
void MultiLevelTimeSeries<VT>::flush()
{
    std::lock_guard<std::mutex> guard(*mMutex);
    if (mCachedCount > 0) {
        for (size_t i = 0; i < mLevels.size(); ++i) {
            mLevels[i].addValueAggregated(mCachedTime, mCachedSum, mCachedCount);
        }
        mCachedCount = 0;
        mCachedSum = 0;
    }
}

template <typename VT>
void MultiLevelTimeSeries<VT>::clear()
{
    std::lock_guard<std::mutex> guard(*mMutex);

    for (auto& level : mLevels) {
        level.clear();
    }

    mCachedTime = TimePoint();
    mCachedSum = 0;
    mCachedCount = 0;
}

#endif //PERFORMANCE_MULTILEVELTIMESERIES_INL_H
