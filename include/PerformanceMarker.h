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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

#include "Defer.h"
#include "TimeseriesHistogram.h"
#include "cpptime.h"

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
    void addFloatValue(const std::string& name, float value) { addValue(name, double(value)); }
    void addIntValue(const std::string& name, int value) { addValue(name, double(value)); }
    void addInt64Value(const std::string& name, int64_t value) { addValue(name, double(value)); }

    // 获取最新的报告
    std::string getLastReport();

private:
    PerformanceMarker() = default;

    void generateJsonFile();

    static PerformanceMarker* mInstance;
    static std::mutex mLock;

    static std::string mPrefix;
    static std::chrono::seconds mDuration;
    CppTime::Timer mTimer;
    std::map<std::string, TimeseriesHistogram<double>> mBuckets;
    std::string mReport;
};

// 给name增加一个采样点
#define SOL2_PERFORMANCE_COUNT(name, n) PerformanceMarker::getInstance().addIntValue(name, n)
#define SOL2_PERFORMANCE_COUNT_ONE(name) SOL2_PERFORMANCE_COUNT(name, 1)
#define SOL2_PERFORMANCE_COUNTF(name, n) \
    PerformanceMarker::getInstance().addFloatValue(name, n)
#define SOL2_PERFORMANCE_COUNT64(name, n) \
    PerformanceMarker::getInstance().addInt64Value(name, n)

// 用于测量一段代码的执行时间，并给 name 增加这个采样点
#define SOL2_PERFORMANCE_MEASURE_HELP(name, startTime)                                                      \
    auto startTime = std::chrono::steady_clock::now();                                                      \
    sol2::Defer timeVar##_Defer_ = [&]() -> void {                                                          \
        auto endTime = std::chrono::steady_clock::now();                                                    \
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count(); \
        SOL2_PERFORMANCE_COUNT(name, duration);                                                             \
    };
#define SOL2_PERFORMANCE_MEASURE(name) SOL2_PERFORMANCE_MEASURE_HELP(name, L_DEFER_COMBINE(_perf_measure_start_, __LINE__));

#endif //PERFORMANCE_PERFORMANCEMARKER_H
