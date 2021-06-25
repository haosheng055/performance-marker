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
#include <mutex>
#include <memory>

#include "BucketedTimeSeries.h"

/*
 * MultiLevelTimeSeries类拥有多个级别时间粒度（即不同duration的BucketedTimeSeries）。
 *
 * 这个类可以很容易地用于同时跟踪数个预定时间段内的数据。例如，可以使用它来同时跟踪过去5分钟、
 * 15分钟、30分钟的数据的count、sum、avg。
 *
 * 该类不是线程安全的--请使用您自己的同步！
 */
template <typename VT>
class MultiLevelTimeSeries {
public:
    using ValueType = VT;
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    using TimePoint = Clock::time_point;
    using Level = BucketedTimeSeries<ValueType>;

    /*
     * 这将创建一个MultiLevelTimeSeries对象，用于跟踪数个不同duration（level）的
     * 时间序列数据。注意：在初始化level时，至少指定一个明确的时间level，同时应该确保
     * 每个level对应的duration是递增的，如{seconds(1),minutes(1),hours(1)}。
     */
    MultiLevelTimeSeries(
        size_t nBuckets, size_t nLevels, const Duration levelDurations[]);

    MultiLevelTimeSeries(
        size_t nBuckets, std::initializer_list<Duration> durations);

    /*
     * 将时间now处的值val添加到所有level。
     *
     * 注意：相同时间点的数据会被加入的缓存中，这些数据暂时不会被写入到
     * level中，直到你主动调用update()/flush()函数、或者一个更新的
     * 数据被添加。
     */
    void addValue(TimePoint now, const ValueType& val);

    /*
     * 在所有level中，添加给定次数的、时间now处的val值。
     */
    void addValue(TimePoint now, const ValueType& val, uint64_t times);

    /*
     * 在所有level中，添加时间now处的数据总和，样本个数为给定值。
     */
    void addValueAggregated(
        TimePoint now, const ValueType& total, uint64_t nsamples);

    /*
     * 将缓存中的数据写入buckets,同时丢弃过时的数据
     */
    void update(TimePoint now);

    /*
     * 返回给定level上跟踪的所有数据的count。
     *
     * 注意:
     * 在获取count信息之前，应该先调用update()或者flush()，用来清除过时的数据，或者
     * 将缓存中的数据写入到每个level中。
     */
    uint64_t count(size_t level) const { return getLevel(level).count(); }

    /*
     * 返回给定level上跟踪的所有数据的sum。
     *
     * 注意:
     * 在获取sum信息之前，应该先调用update()或者flush()，用来清除过时的数据，或者
     * 将缓存中的数据写入到每个level中。
     */
    ValueType sum(size_t level) const { return getLevel(level).sum(); }

    /*
     * 返回给定level上跟踪的所有数据的avg (即sum / count)。
     *
     * 注意:
     * 在获取avg信息之前，应该先调用update()或者flush()，用来清除过时的数据，或者
     * 将缓存中的数据写入到每个level中。
     */
    double avg(size_t level) const { return getLevel(level).avg(); }

    /*
     * 返回给定level上跟踪的所有数据的rate (即sum / elapsed time)。
     *
     * 注意:
     * 在获取rate信息之前，应该先调用update()或者flush()，用来清除过时的数据，或者
     * 将缓存中的数据写入到每个level中。
     */
    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(size_t level) const
    {
        return getLevel(level).template rate<ReturnType, Interval>();
    }

    /*
     * 返回给定level上跟踪的所有数据的countRate (即count / elapsed time)。
     *
     * 注意:
     * 在获取countRate信息之前，应该先调用update()或者flush()，用来清除过时的数据，或者
     * 将缓存中的数据写入到每个level中。
     */
    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(size_t level) const
    {
        return getLevel(level).template countRate<ReturnType, Interval>();
    }

    uint64_t count(Duration duration) const
    {
        return getLevelByDuration(duration).count();
    }

    ValueType sum(Duration duration) const
    {
        return getLevelByDuration(duration).sum();
    }

    double avg(Duration duration) const
    {
        return getLevelByDuration(duration).avg();
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(Duration duration) const
    {
        return getLevelByDuration(duration).template rate<ReturnType, Interval>();
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(Duration duration) const
    {
        return getLevelByDuration(duration)
            .template countRate<ReturnType, Interval>();
    }

    uint64_t count(TimePoint start, TimePoint end) const
    {
        return getLevel(start).count(start, end);
    }

    ValueType sum(TimePoint start, TimePoint end) const
    {
        return getLevel(start).sum(start, end);
    }

    double avg(TimePoint start, TimePoint end) const
    {
        return getLevel(start).avg(start, end);
    }

    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(TimePoint start, TimePoint end) const
    {
        return getLevel(start).template rate<ReturnType, Interval>(start, end);
    }

    size_t numBuckets() const { return mLevels[0].numBuckets();}

    size_t numLevels() const { return mLevels.size(); }

    const Level& getLevel(size_t level) const { return mLevels[level]; }

    const Level& getLevel(TimePoint start) const
    {
        for (Level & level : mLevels) {
            if (level.getLatestTime() - level.getDuration() <= start) {
                return level;
            }
        }
        return mLevels.back();
    }

    const Level& getLevelByDuration(Duration duration) const
    {
        for (const auto& level : mLevels) {
            if (level.getDuration() == duration) {
                return level;
            }
        }
    }

    void clear();

    /*
     * 将缓存中的数据写入buckets
     */
    void flush();

private:
    std::vector<Level> mLevels;
    std::shared_ptr<std::mutex> mMutex;

    // 缓存中存储同样时间的数据，当新时间的数据到来或者调用flush()时，缓存会被清空
    TimePoint mCachedTime;
    ValueType mCachedSum;
    uint64_t mCachedCount;
};

#include "MultiLevelTimeSeries-inl.h"

#endif//PERFORMANCE_MULTILEVELTIMESERIES_H
