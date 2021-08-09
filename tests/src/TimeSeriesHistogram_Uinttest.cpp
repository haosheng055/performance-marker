//
// Created by haosheng on 2021/6/30.
//
#include "TimeseriesHistogram.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <thread>

class TimeseriesHistogramTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        auto beginTime = std::chrono::steady_clock::now();
        auto nextTime = beginTime + std::chrono::seconds(10);
        timeseriesHistogram1.addValue(beginTime, 100);
        timeseriesHistogram1.addValue(nextTime, 1);
        timeseriesHistogram1.addValue(nextTime, 2);
        timeseriesHistogram1.addValue(nextTime, 3);
        timeseriesHistogram1.update(nextTime);
    }
    TimeseriesHistogram<double> timeseriesHistogram1
        { 1000, -1e5,1e5,
            MultiLevelTimeSeries<double>(
                10,{std::chrono::seconds(10), std::chrono::minutes(1)})};
};

TEST_F(TimeseriesHistogramTest, addValueIn10Sec)
{
    EXPECT_EQ(timeseriesHistogram1.count(0), 3);
    EXPECT_EQ(timeseriesHistogram1.sum(0), 6);
    EXPECT_EQ(timeseriesHistogram1.avg(0), 2);
    EXPECT_EQ(timeseriesHistogram1.rate(0), 0.6);
    EXPECT_EQ(timeseriesHistogram1.countRate(0), 0.3);
}

TEST_F(TimeseriesHistogramTest, addValueIn1Min)
{
    EXPECT_EQ(timeseriesHistogram1.count(1), 4);
    EXPECT_EQ(timeseriesHistogram1.sum(1), 106);
    EXPECT_DOUBLE_EQ(timeseriesHistogram1.avg(1), 26.5);
    EXPECT_NEAR(timeseriesHistogram1.rate(1), 10.6, 1);
    EXPECT_NEAR(timeseriesHistogram1.countRate(1), 0.4, 0.1);
}

