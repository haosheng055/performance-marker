/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/23
 *
 */

#ifndef PERFORMANCE_TIMESERIESHISTOGRAM_INL_H
#define PERFORMANCE_TIMESERIESHISTOGRAM_INL_H

template <typename T>
TimeseriesHistogram<T>::TimeseriesHistogram(
    ValueType bucketSize,
    ValueType min,
    ValueType max,
    const ContainerType& defaultContainer)
    : mBuckets(bucketSize, min, max, defaultContainer) {}

template <typename T>
void TimeseriesHistogram<T>::addValue(
    TimePoint now, const ValueType& value) {
    mBuckets.getByValue(value).addValue(now, value);
}

template <typename T>
void TimeseriesHistogram<T>::addValue(
    TimePoint now, const ValueType& value, uint64_t times) {
    mBuckets.getByValue(value).addValue(now, value, times);
}


template <typename T>
T TimeseriesHistogram<T>::getPercentileEstimate(
    double pct, size_t level) const {
    return mBuckets.getPercentileEstimate(
        pct / 100.0, CountFromLevel(level), AvgFromLevel(level));
}

template <typename T>
T TimeseriesHistogram<T>::getPercentileEstimate(
    double pct, TimePoint start, TimePoint end) const {
    return mBuckets.getPercentileEstimate(
        pct / 100.0,
        CountFromInterval(start, end),
        AvgFromInterval<T>(start, end));
}

template <typename T>
size_t TimeseriesHistogram<T>::getPercentileBucketIdx(
    double pct, size_t level) const {
    return mBuckets.getPercentileBucketIdx(pct / 100.0, CountFromLevel(level));
}

template <typename T>
size_t TimeseriesHistogram<T>::getPercentileBucketIdx(
    double pct, TimePoint start, TimePoint end) const {
    return mBuckets.getPercentileBucketIdx(
        pct / 100.0, CountFromInterval(start, end));
}

template <typename T>
void TimeseriesHistogram<T>::clear() {
    for (size_t i = 0; i < mBuckets.getNumBuckets(); i++) {
        mBuckets.getByIndex(i).clear();
    }
}

template <typename T>
void TimeseriesHistogram<T>::update(TimePoint now) {
    for (size_t i = 0; i < mBuckets.getNumBuckets(); i++) {
        mBuckets.getByIndex(i).update(now);
    }
}

template <typename T>
std::string TimeseriesHistogram<T>::getString(size_t level) const {
    std::stringstream result;
    result.setf(std::ios::fixed);
    result << std::setprecision(2);
    result << "\t\t\"count\": " << count(level) << ",\n"
        << "\t\t\"accu\": " << sum(level) << ",\n"
        << "\t\t\"avg\": " << avg(level) << ",\n"
        << "\t\t\"rate\": " << rate(level) << ",\n"
        << "\t\t\"qps\": " << countRate(level) << ",\n"
        << "\t\t\"99%\": " << getPercentileEstimate(99,level) << ",\n"
        << "\t\t\"90%\": " << getPercentileEstimate(90,level) << ",\n"
        << "\t\t\"80%\": " << getPercentileEstimate(80,level);
    return result.str();
}

template <typename T>
std::string TimeseriesHistogram<T>::getString(
    TimePoint start, TimePoint end) const {
    std::string result;

    for (size_t i = 0; i < mBuckets.getNumBuckets(); i++) {
        if (i > 0) {
            result.append(",");
        }
        const ContainerType& cont = mBuckets.getByIndex(i);
        result += "bucketMin: " + std::to_string(mBuckets.getBucketMin(i))
                  + "count: " + std::to_string(cont.count(start,end))
                  + "avg: " + std::to_string(cont.avg(start,end)) + "\n";
    }

    return result;
}


#endif //PERFORMANCE_TIMESERIESHISTOGRAM_INL_H
