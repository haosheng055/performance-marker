/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/16
 *
 */

#ifndef PERFORMANCE_BUCKET_H
#define PERFORMANCE_BUCKET_H

#include <cstdint>

template <typename T>
class Bucket {
public:
    using ValueType = T;

    Bucket(): mSum(ValueType()), mCount(0) {}

    void addValue(const ValueType &value,const uint64_t& count)
    {
        mSum+=value*count;
        mCount+=count;
    }

    void clearBucket()
    {
        mCount = 0;
        mSum = ValueType();
    }

    Bucket& operator+=(const Bucket& bucket)
    {
        addValue(bucket.mSum,bucket.mCount);
        return *this;
    }

    Bucket& operator-=(const Bucket& bucket)
    {
        mSum -= bucket.mSum;
        mCount -= bucket.mCount;
    }

    ValueType avg() const
    {
        if(mCount == 0)
            return ValueType();
        auto sumf = double(mSum);
        auto countf = double(mCount);
        return static_cast<ValueType>(sumf / countf);
    }

    uint64_t mCount;
    ValueType mSum;
};

#endif //PERFORMANCE_BUCKET_H
