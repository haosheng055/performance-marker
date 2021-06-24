/**
 * Copyright (c) 2018 Duobei Brothers Information Technology Co.,Ltd. All rights reserved.
 *
 * Author: haosheng (sheng.hao@duobei.com)
 *
 * Date: 2021/6/23
 *
 */

#ifndef PERFORMANCE_PERFORMANCEMARKER_H
#define PERFORMANCE_PERFORMANCEMARKER_H

#include <string>
#include <map>
#include <iostream>

#include "TimeseriesHistogram.h"
#include "cpptime.h"

// TODO:addValue不是异步的
// TODO:线程不安全
class PerformanceMarker {
public:

    static PerformanceMarker& getInstance();

    /*
     * 该函数会在程序启动时调用。
     *
     * 会启动一个线程，用来定时打印报告。
     */
    static void initialize(const std::string& prefix, uint32_t intervalSeconds);

    // 向内部增加一个采样点value
    void addValue(const std::string& name, double value);
    void addFloatValue(const std::string& name, float value) { addValue(name,double(value)); }
    void addIntValue(const std::string& name, int value) { addValue(name,double(value)); }
    void addInt64Value(const std::string& name, int64_t value) { addValue(name,double(value)); }

    // 获取最新的报告
    static const std::string& getLastReport();

private:
    PerformanceMarker() = default;

    static PerformanceMarker* mInstance;

    static std::string mPrefix;
    static std::chrono::seconds mDuration;
    CppTime::Timer mTimer;
    std::map<std::string, TimeseriesHistogram<double>> mBuckets;
    std::string mReport;
};

// 以下4个接口给name增加一个值n
#define SOL2_PERFORMANCE_COUNT(name, n) PerformanceMarker::getInstance().addIntValue(name, n)
#define SOL2_PERFORMANCE_COUNT_ONE(name) PERFORMANCE_COUNT(name, 1)
#define SOL2_PERFORMANCE_COUNTF(name, n) PerformanceMarker::getInstance().addFloatValue(name, n)
#define SOL2_PERFORMANCE_COUNT64(name, n) PerformanceMarker::getInstance().addInt64Value(name, n)


#endif //PERFORMANCE_PERFORMANCEMARKER_H
