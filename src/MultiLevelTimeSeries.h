/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/16
 *
 */

#ifndef PERFORMANCE_MULTILEVELTIMESERIES_H
#define PERFORMANCE_MULTILEVELTIMESERIES_H

#include <chrono>
#include <vector>

#include "BucketedTimeSeries.h"

template <typename VT, typename CT = std::chrono::steady_clock >
class MultiLevelTimeSeries {
public:
    using ValueType = VT;
    using Clock = CT;
    using Duration = typename Clock::duration;
    using TimePoint = typename Clock::time_point;
    using Level = BucketedTimeSeries<ValueType, Clock>;

    /*
     * 这将创建一个新的MultiLevelTimeSeries，用于跟踪数个不同duration（level）的BucketTimeSeries
     * 的时间序列数据。为了提高存储效率，在每个级别跟踪的时间序列数据进一步除以numBuckets。
     */
    MultiLevelTimeSeries(
        size_t numBuckets, size_t numLevels, const Duration levelDurations[]);

    MultiLevelTimeSeries(
        size_t numBuckets, std::initializer_list<Duration> durations);

    size_t numBuckets() const {
        // The constructor ensures that levels_ has at least one item
        return levels_[0].numBuckets();
    }

    size_t numLevels() const { return levels_.size(); }

    const Level& getLevel(size_t level) const {
        return levels_[level];
    }

    const Level& getLevel(TimePoint start) const {
        for (Level & level : levels_) {
            if (level.isAllTime()) {
                return level;
            }
            if (level.getLatestTime() - level.getDuration() <= start) {
                return level;
            }
        }
        return levels_.back();
    }

    const Level& getLevelByDuration(Duration duration) const {
        // since the number of levels is expected to be small (less than 5 in most
        // cases), a simple linear scan would be efficient and is intentionally
        // chosen here over other alternatives for lookup.
        for (const auto& level : levels_) {
            if (level.getDuration() == duration) {
                return level;
            }
        }
    }

    ValueType sum(size_t level) const { return getLevel(level).sum(); }

    template <typename ReturnType = double>
    ReturnType avg(size_t level) const {
        return getLevel(level).avg();
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(size_t level) const {
        return getLevel(level).template rate<ReturnType, Interval>();
    }

    uint64_t count(size_t level) const { return getLevel(level).count(); }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(size_t level) const {
        return getLevel(level).template countRate<ReturnType, Interval>();
    }

    ValueType sum(Duration duration) const {
        return getLevelByDuration(duration).sum();
    }

    template <typename ReturnType = double>
    ReturnType avg(Duration duration) const {
        return getLevelByDuration(duration).avg();
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(Duration duration) const {
        return getLevelByDuration(duration).template rate<ReturnType, Interval>();
    }

    uint64_t count(Duration duration) const {
        return getLevelByDuration(duration).count();
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(Duration duration) const {
        return getLevelByDuration(duration)
            .template countRate<ReturnType, Interval>();
    }

    ValueType sum(TimePoint start, TimePoint end) const {
        return getLevel(start).sum(start, end);
    }

    template <typename ReturnType = double>
    ReturnType avg(TimePoint start, TimePoint end) const {
        return getLevel(start).template avg<ReturnType>(start, end);
    }

    template <typename ReturnType = double>
    ReturnType rate(TimePoint start, TimePoint end) const {
        return getLevel(start).template rate<ReturnType>(start, end);
    }

    uint64_t count(TimePoint start, TimePoint end) const {
        return getLevel(start).count(start, end);
    }

    void addValue(TimePoint now, const ValueType& val);

    void addValue(TimePoint now, const ValueType& val, uint64_t times);

    void addValueAggregated(
        TimePoint now, const ValueType& total, uint64_t nsamples);

    /*
     * 将缓存中的数据写入buckets,同时将所有的Levels更新到传入的时间,丢弃过时的数据
     */
    void update(TimePoint now);

    void clear();

    /*
     * 将缓存中的数据写入buckets
     */
    void flush();

private:
    std::vector<Level> levels_;

    // 缓存中存储同样时间的数据，当新时间的数据到来或者调用flush()时
    // 缓存会被清空
    TimePoint cachedTime_;
    ValueType cachedSum_;
    uint64_t cachedCount_;
};



#endif//PERFORMANCE_MULTILEVELTIMESERIES_H
