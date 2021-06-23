//
// Created by DELL on 2021/6/23.
//

#ifndef PERFORMANCE_HISTOGRAMBUCKETS_INL_H
#define PERFORMANCE_HISTOGRAMBUCKETS_INL_H

template <typename T>
HistogramBuckets<T>::HistogramBuckets(
    ValueType bucketSize,
    ValueType min,
    ValueType max,
    const BucketType& defaultBucket)
    : bucketSize_(bucketSize), min_(min), max_(max)
{

    int64_t numBuckets = int64_t((max - min) / bucketSize);
    // 如果不整除，再加一个bucket
    if (numBuckets * bucketSize < max - min) {
        ++numBuckets;
    }
    // 增加两个额外的bucket，一个负责小于min的值，另一个负责大于max的值
    numBuckets += 2;
    buckets_.assign(size_t(numBuckets), defaultBucket);
}

template <typename T>
size_t HistogramBuckets<T>::getBucketIdx(ValueType value) const
{
    if (value < min_) {
        return 0;
    } else if (value >= max_) {
        return buckets_.size() - 1;
    } else {
        // the 1 is the below_min bucket
        return size_t(((value - min_) / bucketSize_) + 1);
    }
}

template <typename T>
template <typename CountFn>
uint64_t HistogramBuckets<T>::computeTotalCount(
    CountFn countFromBucket) const
{
    uint64_t count = 0;
    for (size_t n = 0; n < buckets_.size(); ++n) {
        count += countFromBucket(const_cast<const BucketType&>(buckets_[n]));
    }
    return count;
}

template <typename T>
template <typename CountFn>
size_t HistogramBuckets<T>::getPercentileBucketIdx(
    double pct,
    CountFn countFromBucket,
    double* lowPct,
    double* highPct) const
{

    auto numBuckets = buckets_.size();

    // 计算出每个bucket中数据的count
    std::vector<uint64_t> counts(numBuckets);
    uint64_t totalCount = 0;
    for (size_t n = 0; n < numBuckets; ++n) {
        uint64_t bucketCount =
            countFromBucket(const_cast<const BucketType&>(buckets_[n]));
        bucketCount = buckets_[n].count(1);
        counts[n] = bucketCount;
        totalCount += bucketCount;
    }

    // 如果所有bucket都没有数据，返回最小的bucket下标即可。
    if (totalCount == 0) {
        // lowPct和highPct设为0，代表bucket中没有数据。
        if (lowPct) {
            *lowPct = 0.0;
        }
        if (highPct) {
            *highPct = 0.0;
        }
        return 1;
    }

    // 循环遍历每个bucket, 同时跟踪每个bucket所占的count的范围，例如[0,10%],[10%,17%]
    // 这样，如果遍历到一个范围包含了我们给定的pct，返回这个bucket下标即可。
    double prevPct = 0.0;
    double curPct = 0.0;
    uint64_t curCount = 0;
    size_t idx;
    for (idx = 0; idx < numBuckets; ++idx) {
        if (counts[idx] == 0) {
            continue;
        }

        prevPct = curPct;
        curCount += counts[idx];
        curPct = static_cast<double>(curCount) / totalCount;
        if (pct <= curPct) {
            // 找到第一个右边界比pct大的bucket
            break;
        }
    }

    if (lowPct) {
        *lowPct = prevPct;
    }
    if (highPct) {
        *highPct = curPct;
    }
    return idx;
}

template <typename T>
template <typename CountFn, typename AvgFn>
T HistogramBuckets<T>::getPercentileEstimate(
    double pct, CountFn countFromBucket, AvgFn avgFromBucket) const
{
    // 先找到给定pct落入的bucket
    double lowPct;
    double highPct;
    size_t bucketIdx =
        getPercentileBucketIdx(pct, countFromBucket, &lowPct, &highPct);
    if (lowPct == 0.0 && highPct == 0.0) {
        return ValueType();
    }
    if (lowPct == highPct) {
        // Unlikely to have exact equality,
        // but just return the bucket average in this case.
        // We handle this here to avoid division by 0 below.
        return avgFromBucket(buckets_[bucketIdx]);
    }

    // 计算这个bucket 存储数据的平均值、最大值、最小值。
    ValueType avg = avgFromBucket(buckets_[bucketIdx]);
    ValueType low;
    ValueType high;
    if (bucketIdx == 0) {
        high = min_;
        low = high - (2 * (high - avg));
        // Adjust low in case it wrapped
        if (low > avg) {
            low = std::numeric_limits<ValueType>::min();
        }
    } else if (bucketIdx == buckets_.size() - 1) {
        low = max_;
        high = low + (2 * (avg - low));
        // Adjust high in case it wrapped
        if (high < avg) {
            high = std::numeric_limits<ValueType>::max();
        }
    } else {
        low = getBucketMin(bucketIdx);
        high = getBucketMax(bucketIdx);
    }

    // 由于无法得知bucket中所有数据的具体value，于是我们假设bucket中的数据是均匀分布的，
    // 中位数的value即为avg平均值。
    double medianPct = (lowPct + highPct) / 2.0;
    if (pct < medianPct) {
        double pctThroughSection = (pct - lowPct) / (medianPct - lowPct);
        return T(low + ((avg - low) * pctThroughSection));
    } else {
        double pctThroughSection = (pct - medianPct) / (highPct - medianPct);
        return T(avg + ((high - avg) * pctThroughSection));
    }
}

#endif //PERFORMANCE_HISTOGRAMBUCKETS_INL_H
