//
// Created by Hao sheng on 2021/6/30.
//
#include "../../include/BucketedTimeSeries.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <thread>

class BucketedTimeSeriesTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        auto beginTime = std::chrono::steady_clock::now();
        bucketedTimeSeries1.addValue(beginTime, 10000);
        bucketedTimeSeries1.addValue(beginTime + std::chrono::seconds(10), 1);
        bucketedTimeSeries1.addValue(beginTime + std::chrono::seconds(11), 2);
        bucketedTimeSeries1.addValue(beginTime + std::chrono::seconds(12), 3);
    }
    BucketedTimeSeries<double> bucketedTimeSeries1 { 10, std::chrono::seconds(10) };
};

TEST_F(BucketedTimeSeriesTest, addValue)
{
    EXPECT_EQ(bucketedTimeSeries1.count(), 3);
    EXPECT_EQ(bucketedTimeSeries1.sum(), 6);
    EXPECT_EQ(bucketedTimeSeries1.avg(), 2);
    EXPECT_EQ(bucketedTimeSeries1.rate(), 0.6);
    EXPECT_EQ(bucketedTimeSeries1.countRate(), 0.3);
}
