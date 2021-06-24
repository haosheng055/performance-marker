//
// Created by DELL on 2021/6/23.
//
#include "PerformanceMarker.h"

using namespace std;

PerformanceMarker* PerformanceMarker::mInstance = nullptr;
chrono::seconds PerformanceMarker::mDuration = chrono::seconds();
string PerformanceMarker::mPrefix = string();

void PerformanceMarker::initialize(const std::string& prefix, uint32_t intervalSeconds)
{
    mPrefix = prefix;
    mDuration = std::chrono::seconds(intervalSeconds);
    PerformanceMarker::getInstance();
}

PerformanceMarker & PerformanceMarker::getInstance()
{
    if(mInstance == nullptr)
    {
        mInstance = new PerformanceMarker();
        mInstance->mTimer.add(
            std::chrono::steady_clock::now(),
            [&](CppTime::timer_id id) -> void {
                for (auto& bucket : mInstance->mBuckets) {
                    bucket.second.update(chrono::steady_clock::now());
                    mInstance->mReport += mPrefix + "_" +
                                          bucket.first + ":\n" + bucket.second.getString(0);
                }
                mInstance->mReport += "\n";
                std::cout << mInstance->mReport;
                mInstance->mReport = "";
            },
            mDuration);
    }
    return *mInstance;
}

void PerformanceMarker::addValue(const std::string& name, double value)
{
    if(mBuckets.count(name) == 0){
        TimeseriesHistogram<double> timeseriesHistogram(
            1e3,-1e5,1e5,MultiLevelTimeSeries<double>
                (100,{mDuration}));
        mBuckets.insert(pair<string,TimeseriesHistogram<double>>(name, timeseriesHistogram));
    }
    (mBuckets.find(name)->second).addValue(chrono::steady_clock::now(),value);
}

const std::string & PerformanceMarker::getLastReport()
{
    return getInstance().mReport;
}