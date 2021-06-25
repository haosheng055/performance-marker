/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/22
 *
 */

#ifndef PERFORMANCE_HISTOGRAMBUCKETS_H
#define PERFORMANCE_HISTOGRAMBUCKETS_H

#include "MultiLevelTimeSeries.h"

/*
 * TimeSeriesHistogram的帮助类
 */
template <typename T>
class HistogramBuckets {
public:
    using ValueType = T;
    using BucketType = MultiLevelTimeSeries<ValueType>;

    /*
     * 创建一组直方图buckets的集合。BucketType的类型为MultiLevelTimeSeries<T>
     *
     * 为每个间隔创建一个bucket，一个bucket存储bucketSize范围大小的数据。
     * 此外，将创建一个bucket来跟踪 < min 的所有值，以及一个bucket跟踪 > max 的所有值。
     *
     * 如果 (max - min) 不能被bucketSize整除，最后一个bucket覆盖的值范围会比其他bucket小一些
     *
     * (max - min) 必须>= bucketSize.
     */
    HistogramBuckets(
        ValueType bucketSize,
        ValueType min,
        ValueType max,
        const BucketType& defaultBucket);

    /* 返回每个bucket负责的范围宽度 */
    ValueType getBucketSize() const { return mBucketSize; }

    ValueType getMin() const { return mMin; }

    ValueType getMax() const { return mMax; }

    /*
     * 返回buckets的数目
     *
     * 这包括负责[min,max)范围的bucket以及额外的两个bucket
     */
    size_t getNumBuckets() const { return mBuckets.size(); }

    /* 返回给定的value值落入的bucket下标 */
    size_t getBucketIdx(ValueType value) const;

    /* 返回给定的value值落入的bucket */
    BucketType& getByValue(ValueType value) {
        return mBuckets[getBucketIdx(value)];
    }

    /* 返回给定的value值落入的bucket */
    const BucketType& getByValue(ValueType value) const {
        return mBuckets[getBucketIdx(value)];
    }

    /*
     * 返回给定下标值的bucket
     *
     * 注意：下标为0的bucket负责处理比min小的值，而下标为1的bucket是负责给定范围的
     * 第一个bucket。
     */
    BucketType& getByIndex(size_t idx) { return mBuckets[idx]; }

    const BucketType& getByIndex(size_t idx) const { return mBuckets[idx]; }

    /*
     * 返回给定index处bucket的左边界。
     *
     * 注意：每个bucket存储的值范围要么是[bucketMin,bucketMin+bucketSize)，要么是
     * [bucketMin,max)。
     */
    ValueType getBucketMin(size_t idx) const
    {
        if (idx == 0) {
            return std::numeric_limits<ValueType>::min();
        }
        if (idx == mBuckets.size() - 1) {
            return mMax;
        }

        return ValueType(mMin + ((idx - 1) * mBucketSize));
    }

    /*
     * 返回给定index处bucket的右边界。
     *
     * 注意：每个bucket存储的值范围要么是[bucketMin,bucketMin+bucketSize)，要么是
     * [bucketMin,max)。
     */
    ValueType getBucketMax(size_t idx) const {
        if (idx == mBuckets.size() - 1) {
            return std::numeric_limits<ValueType>::max();
        }

        return ValueType(mMin + (idx * mBucketSize));
    }

    /*
     * 计算所有buckets（所有MultiLevelTimeSeries）中数据的counts
     *
     *
     * @param countFn 一个函数，以一个const BcucketType& 对象作为参数，返回值是这个bucket
     *                中数据的count。
     */
    template <typename CountFn>
    uint64_t computeTotalCount(CountFn countFromBucket) const;

    /*
     * 计算给定percent的数据落入的bucket下标，以及该bucket的信息
     *
     * 通过累加每个bucket中数据的count，先算出totalCount，然后计算落在哪里
     *
     * @param pct     目标百分数，范围是0.0-1.0
     * @param countFn 一个函数，以一个const BcucketType& 对象作为参数，返回值是这个bucket
     *                中数据的count。
     * @param lowPct  找到的bucket所占数据个数百分比的下界
     * @param highPct 找到的bucket所占数据个数百分比的下界
     *
     * @return 返回给定percent的数据落入的bucket下标
     */
    template <typename CountFn>
    size_t getPercentileBucketIdx(
        double pct,
        CountFn countFromBucket,
        double* lowPct = nullptr,
        double* highPct = nullptr) const;

    /*
     * 计算给定percent的数据的value是多少。
     *
     * 先使用getPercentileBucketIdx获取落在哪个bucket上，由于无法得知bucket中
     * 所有数据的具体value，于是使用中位数找到一个index，然后假设中位数的value
     * 也是avg、以及从low->median的数据分布都是平均的，来计算返回值。
     *
     * @param pct     目标百分数，范围是0.0-1.0
     * @param countFn 一个函数，以一个 const BcucketType& 对象作为参数，返回值是这个bucket
     *                对象中数据的count。
     * @param avgFn   一个函数，以一个 const BcucketType& 对象作为参数，返回值是这个bucket
     *                对象中数据的avg。
     *
     * @return 返回一个数据value，表示有占比为pct%的数据都比找到的value小。
     */
    template <typename CountFn, typename AvgFn>
    ValueType getPercentileEstimate(
        double pct, CountFn countFromBucket, AvgFn avgFromBucket) const;

    /*
     * buckets的迭代器
     *
     * 注意：下标为0的bucket负责处理比min小的值，而下标为1的bucket是负责给定范围的
     * 第一个bucket。
     */
    typename std::vector<BucketType>::const_iterator begin() const {
        return mBuckets.begin();
    }
    typename std::vector<BucketType>::iterator begin() {
        return mBuckets.begin();
    }
    typename std::vector<BucketType>::const_iterator end() const {
        return mBuckets.end();
    }
    typename std::vector<BucketType>::iterator end() { return mBuckets.end(); }

private:
    ValueType mBucketSize;
    ValueType mMin;
    ValueType mMax;
    std::vector<BucketType> mBuckets;
};

#include "HistogramBuckets-inl.h"

#endif //PERFORMANCE_HISTOGRAMBUCKETS_H
