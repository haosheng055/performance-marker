//
// Created by haosheng on 2021/6/30.
//
#include "MultiLevelTimeSeries.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

class MultiLevelTimeSeriesTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        auto beginTime = std::chrono::steady_clock::now();
        auto nextTime = beginTime + std::chrono::seconds(10);
        multiLevelTimeSeries1.addValue(beginTime, 1000);
        multiLevelTimeSeries1.addValue(nextTime, 1);
        multiLevelTimeSeries1.addValue(nextTime, 2);
        multiLevelTimeSeries1.addValue(nextTime, 3);
        multiLevelTimeSeries1.update(nextTime);
    }
    MultiLevelTimeSeries<double> multiLevelTimeSeries1 { 10, { std::chrono::seconds(10), std::chrono::minutes(1) } };
};

TEST_F(MultiLevelTimeSeriesTest, addValueIn10Sec)
{
    EXPECT_EQ(multiLevelTimeSeries1.count(0), 3);
    EXPECT_EQ(multiLevelTimeSeries1.sum(0), 6);
    EXPECT_EQ(multiLevelTimeSeries1.avg(0), 2);
    EXPECT_EQ(multiLevelTimeSeries1.rate(0), 0.6);
    EXPECT_EQ(multiLevelTimeSeries1.countRate(0), 0.3);
}

TEST_F(MultiLevelTimeSeriesTest, addValueIn1Min)
{
    EXPECT_EQ(multiLevelTimeSeries1.count(1), 4);
    EXPECT_EQ(multiLevelTimeSeries1.sum(1), 1006);
    EXPECT_EQ(multiLevelTimeSeries1.avg(1), 251.5);
}
