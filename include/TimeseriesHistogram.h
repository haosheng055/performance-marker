/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/22
 *
 */

#ifndef PERFORMANCE_TIMESERIESHISTOGRAM_H
#define PERFORMANCE_TIMESERIESHISTOGRAM_H

#include <string>
#include <iomanip>
#include <sstream>

#include "HistogramBuckets.h"
#include "MultiLevelTimeSeries.h"

/*
 * TimeseriesHistogram 跟踪一段时间的数据分布。
 *
 * 具体来说，它是一个bucket直方图，使用数个容器跟踪一个[min,max]的数据范围，每个容器被分配
 * 同样宽度的值范围。每个bucket中都有一个MultiLevelTimeSeries。
 *
 * 例如，这可以解决：“最后一分钟的数据分布是什么？”，该类还可以估计百分位数，并回答：
 * “过去10分钟内第70百分位数的数据值是多少？”
 *
 * 注意：根据bucket的大小和数据分布的平滑程度，估计值可能与实际值相差很大。特别是，如果
 * 给定的百分位落在桶范围之外（例如你的桶范围在0-100000之间，但第99百分位在115000左右），这个
 * 估计可能是非常错误的。
 *
 * 如果bucket数目为n，一般情况下的内存使用量约为3k*（n）。所有的插入操作都分摊到O（1），
 * 所有的查询都是O（n）。
 */
template <typename VT>
class TimeseriesHistogram {
public:
    using ValueType = VT;
    using ContainerType = MultiLevelTimeSeries<VT>;
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    using TimePoint = Clock::time_point;

    /*
     * 创建一个TimeSeries直方图并初始化buckets和levels。
     *
     * 每个buckets负责bucketSize的范围，所有的bucket将[min，max)范围内的值平均划分，最后一个
     * bucket可能更短。
     *
     * 有两个buckets总是额外被创建，bucket(-inf，min)和bucket[max，+inf]。
     *
     * @param bucketSize 每个bucket负责的值的宽度
     * @param min 数据范围的最小值
     * @param max 数据范围的最大值
     * @param defaultContainer 一个提前初始化的TimeSeries容器，需要给定容器level，例如
     *                         MutilevelTimeSeries<T> buck(bucketNum,{chrono:seconds(1)})
     */
    TimeseriesHistogram(
        ValueType bucketSize,
        ValueType min,
        ValueType max,
        const ContainerType& defaultContainer);

    /*
     * 使用给定的时间戳 now 来更新底层的数据对象。
     *
     * 注意：在调用所有的查询函数之前，都应该调用此update函数，否则可能会使用过时的数据。
     */
    void update(TimePoint now);

    void clear();

    /* 向某个bucket中，添加时间now处的值value。 */
    void addValue(TimePoint now, const ValueType& value);
    /* 向某个bucket中，添加给定次数的、时间now处的值value。 */
    void addValue(TimePoint now, const ValueType& value, uint64_t times);

    /* 返回给定时间level中的数据count（所有bucket） */
    uint64_t count(size_t level) const {
        uint64_t total = 0;
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            total += mBuckets.getByIndex(b).count(level);
        }
        return total;
    }

    /* 返回给定时间范围中的数据count（所有bucket）。  */
    uint64_t count(TimePoint start, TimePoint end) const {
        uint64_t total = 0;
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            total += mBuckets.getByIndex(b).count(start, end);
        }
        return total;
    }

    /* 返回给定时间等级中的数据sum（所有bucket中）。 */
    ValueType sum(size_t level) const {
        ValueType total = ValueType();
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            total += mBuckets.getByIndex(b).sum(level);
        }
        return total;
    }

    /* 返回给定时间范围中的数据sum（所有bucket）。 */
    ValueType sum(TimePoint start, TimePoint end) const {
        ValueType total = ValueType();
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            total += mBuckets.getByIndex(b).sum(start, end);
        }
        return total;
    }

    /* 返回给定时间等级中的数据avg（所有bucket）。 */
    template <typename ReturnType = double>
    ReturnType avg(size_t level) const {
        auto total = ValueType();
        uint64_t nsamples = 0;
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            total += mBuckets.getByIndex(b).sum(level);
            nsamples += mBuckets.getByIndex(b).count(level);
        }
        if(nsamples == 0){
            return ReturnType();
        }
        return static_cast<ReturnType>(total / nsamples);
    }

    /* 返回给定时间范围中的数据avg（所有bucket）。*/
    template <typename ReturnType = double>
    ReturnType avg(TimePoint start, TimePoint end) const {
        auto total = ValueType();
        uint64_t nsamples = 0;
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            total += mBuckets.getByIndex(b).sum(start,end);
            nsamples += mBuckets.getByIndex(b).count(start,end);
        }
        if(nsamples == 0){
            return ReturnType();
        }
        return static_cast<ReturnType>(total / nsamples);
    }

    /*
     * 返回给定时间level中的数据rate（所有bucket）。
     * 实际上是sum / elapsed，单位是value per second
     */
    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(size_t level) const {
        auto total = ValueType();
        Interval elapsed(0);
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            const auto& levelObj = mBuckets.getByIndex(b).getLevel(level);
            total += levelObj.sum();
            elapsed = std::max(elapsed, levelObj.template elapsed<Interval>());
        }
        if(elapsed == Interval(0)){
            return ReturnType();
        }
        return ReturnType(total * 1.0 / elapsed.count());
    }

    /*
     * 返回给定时间范围中的数据rate（所有bucket）。
     * 实际上是sum / elapsed，单位是value per second
     */
    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType rate(TimePoint start, TimePoint end) const {
        auto total = ValueType();
        Interval elapsed(0);
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            const auto& levelObj = mBuckets.getByIndex(b).getLevel(start);
            total += levelObj.sum(start,end);
            elapsed = std::max(elapsed, levelObj.template elapsed<Interval>(start,end));
        }
        if(elapsed == Interval(0)){
            return ReturnType();
        }
        return ReturnType(total * 1.0 / elapsed.count());
    }

    /*
     * 返回给定时间level中的数据countRate（所有bucket）。
     * 实际上是count / elapsed，单位是count per second
     */
    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(size_t level) const {
        auto total = count(level);
        Interval elapsed(0);
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            const auto& levelObj = mBuckets.getByIndex(b).getLevel(level);
            elapsed = std::max(elapsed, levelObj.template elapsed<Interval>());
        }
        if(elapsed == Interval(0)){
            return ReturnType();
        }
        return ReturnType(total * 1.0 / elapsed.count());
    }

    /*
     * 返回给定时间范围中的数据的countRate（所有bucket）。
     * 实际上是count / elapsed，单位是count per second
     */
    template <typename ReturnType = double, typename Interval = std::chrono::seconds>
    ReturnType countRate(TimePoint start, TimePoint end) const {
        auto total = count(start,end);
        Interval elapsed(0);
        for (size_t b = 0; b < mBuckets.getNumBuckets(); ++b) {
            const auto& levelObj = mBuckets.getByIndex(b).getLevel(start);
            elapsed = std::max(elapsed, levelObj.template elapsed<Interval>(start,end));
        }
        if(elapsed == Interval(0)){
            return ReturnType();
        }
        return ReturnType(total * 1.0 / elapsed.count());
    }

    /*
     * 返回给定level中，给定百分位数处的值（例如，90%的数据都小于这个value）。
     *
     * 估计值计算方法如下：
     * - （1）在给定时间level上，记录每个bucket中数据的count。
     * - （2）通过count确定给定给定百分位落在哪个bucket中。
     * - （3）假设桶中数据是均匀分布的，以平均数（假设也是中位数的值）作为参考值，
     * 计算出给定百分位处的值。
     *
     * 注意：如果没有数据，此函数会返回ValueType()，一般是0。
     */
    ValueType getPercentileEstimate(double pct, size_t level) const;

    /*
     * 返回给定时间范围中，给定百分位数处的值
     */
    ValueType getPercentileEstimate(
        double pct, TimePoint start, TimePoint end) const;

    /*
     * 对于给定的时间level，输出每个bucket的统计信息，
     * 类似于: bucketMin:-- count:-- avg:--
     */
    std::string getString(size_t level) const;

    /*
     * 对于给定的时间范围，输出每个bucket的统计信息，
     */
    std::string getString(TimePoint start, TimePoint end) const;

    /* 返回每个bucket负责范围的宽度 */
    ValueType getBucketSize() const { return mBuckets.getBucketSize(); }

    ValueType getMin() const { return mBuckets.getMin(); }

    ValueType getMax() const { return mBuckets.getMax(); }

    size_t getNumLevels() const { return mBuckets.getByIndex(0).numLevels(); }

    /* 返回buckets的数目 */
    size_t getNumBuckets() const { return mBuckets.getNumBuckets(); }

    /*
     * 返回给定下标对应bucket的下边界值
     */
    ValueType getBucketMin(size_t bucketIdx) const {
        return mBuckets.getBucketMin(bucketIdx);
    }

    /* 返回给定下标对应bucket */
    const ContainerType& getBucket(size_t bucketIdx) const {
        return mBuckets.getByIndex(bucketIdx);
    }

    /*
     * 在给定的level下，返回给定percent的数据落入的bucket下标，这可以用来获取该bucket的信息
     */
    size_t getPercentileBucketIdx(double pct, size_t level) const;
    /*
     * 在给定的时间范围下，返回给定percent的数据落入的bucket下标，这可以用来获取该bucket的信息
     */
    size_t getPercentileBucketIdx(
        double pct, TimePoint start, TimePoint end) const;

    /* 在给定的level下，获取给定percent的数据落入的bucket下边界值 */
    ValueType getPercentileBucketMin(double pct, size_t level) const {
        return getBucketMin(getPercentileBucketIdx(pct, level));
    }
    /* 在给定的时间范围下，获取给定percent的数据落入的bucket下边界值 */
    ValueType getPercentileBucketMin(
        double pct, TimePoint start, TimePoint end) const {
        return getBucketMin(getPercentileBucketIdx(pct, start, end));
    }

private:
    struct CountFromLevel {
        explicit CountFromLevel(size_t level) : level_(level) {}

        uint64_t operator()(const ContainerType& bucket) const {
            return bucket.count(level_);
        }

    private:
        size_t level_;
    };
    struct CountFromInterval {
        explicit CountFromInterval(TimePoint start, TimePoint end)
            : start_(start), end_(end) {}

        uint64_t operator()(const ContainerType& bucket) const {
            return bucket.count(start_, end_);
        }

    private:
        TimePoint start_;
        TimePoint end_;
    };

    struct AvgFromLevel {
        explicit AvgFromLevel(size_t level) : level_(level) {}

        ValueType operator()(const ContainerType& bucket) const {
            return bucket.avg(level_);
        }

    private:
        size_t level_;
    };

    template <typename ReturnType>
    struct AvgFromInterval {
        explicit AvgFromInterval(TimePoint start, TimePoint end)
            : start_(start), end_(end) {}

        ReturnType operator()(const ContainerType& bucket) const {
            return bucket.template avg(start_, end_);
        }

    private:
        TimePoint start_;
        TimePoint end_;
    };

    HistogramBuckets<ValueType> mBuckets;
};

#include "TimeseriesHistogram-inl.h"

#endif //PERFORMANCE_TIMESERIESHISTOGRAM_H
