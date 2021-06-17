//
// Created by DELL on 2021/6/16.
//
#include "MultiLevelTimeSeries.h"


template <typename VT, typename CT>
MultiLevelTimeSeries<VT, CT>::MultiLevelTimeSeries(
    size_t nBuckets, size_t nLevels, const Duration levelDurations[])
    : cachedTime_(), cachedSum_(0), cachedCount_(0) {

    levels_.reserve(nLevels);
    for (size_t i = 0; i < nLevels; ++i) {
        levels_.emplace_back(nBuckets, levelDurations[i]);
    }
}

template <typename VT, typename CT>
MultiLevelTimeSeries<VT, CT>::MultiLevelTimeSeries(
    size_t nBuckets, std::initializer_list<Duration> durations)
    : cachedTime_(), cachedSum_(0), cachedCount_(0) {

    levels_.reserve(durations.size());
    size_t i = 0;
    Duration prev{0};
    for (auto dur : durations) {
        levels_.emplace_back(nBuckets, dur);
        prev = dur;
        i++;
    }
}

template <typename VT, typename CT>
void MultiLevelTimeSeries<VT, CT>::addValue(
    TimePoint now, const ValueType& val) {
    addValue(now, val, 1);
}

template <typename VT, typename CT>
void MultiLevelTimeSeries<VT, CT>::addValue(
    TimePoint now, const ValueType& val, uint64_t times) {
    addValueAggregated(now,val * times,times);
}

template <typename VT, typename CT>
void MultiLevelTimeSeries<VT, CT>::addValueAggregated(
    TimePoint now, const ValueType& total, uint64_t nsamples) {
    // 如果已经过了一段时间，需要将旧有缓存中的数据写入buckets
    if (cachedTime_ != now) {
        flush();
        cachedTime_ = now;
    }
    // 将传入的数据写入缓存
    cachedSum_ += total;
    cachedCount_ += nsamples;
}

template <typename VT, typename CT>
void MultiLevelTimeSeries<VT, CT>::update(TimePoint now) {
    flush();
    for (size_t i = 0; i < levels_.size(); ++i) {
        levels_[i].update(now);
    }
}

template <typename VT, typename CT>
void MultiLevelTimeSeries<VT, CT>::flush() {
    if (cachedCount_ > 0) {
        for (size_t i = 0; i < levels_.size(); ++i) {
            levels_[i].addValueAggregated(cachedTime_, cachedSum_, cachedCount_);
        }
        cachedCount_ = 0;
        cachedSum_ = 0;
    }
}

template <typename VT, typename CT>
void MultiLevelTimeSeries<VT, CT>::clear() {
    for (auto& level : levels_) {
        level.clear();
    }

    cachedTime_ = TimePoint();
    cachedSum_ = 0;
    cachedCount_ = 0;
}
