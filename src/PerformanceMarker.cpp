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

PerformanceMarker& PerformanceMarker::getInstance()
{
    if (mInstance == nullptr) {
        mInstance = new PerformanceMarker();
        mInstance->mTimer.add(
            std::chrono::steady_clock::now(),
            [&](CppTime::timer_id id) -> void {
                // 生成json格式数据
                mInstance->mReport += "{\n";
                int idx = 0;
                for (auto& bucket : mInstance->mBuckets) {
                    // 清除bucket中过时数据
                    bucket.second.update(chrono::steady_clock::now());
                    mInstance->mReport.append("\t\"" + mPrefix + "_" + bucket.first + "\": {\n" + bucket.second.getString(0) + "\n\t}");
                    idx == mInstance->mBuckets.size() - 1 ? mInstance->mReport.append("\n") : mInstance->mReport.append(",\n");
                    idx++;
                }
                mInstance->mReport += "}";

                // 打开对应日期时间的文件，并写入数据
                auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::stringstream ss;
                ss << std::put_time(std::localtime(&currentTime), "%Y.%m.%d %H.%M.json");
                ofstream outfile;
                outfile.open(R"(.\)" + ss.str());
                if (outfile) {
                    outfile << mInstance->mReport;
                    outfile.close();
                }
                mInstance->mReport = "";
            },
            mDuration);
    }
    return *mInstance;
}

void PerformanceMarker::addValue(const std::string& name, double value)
{
    if (mBuckets.count(name) == 0) {
        TimeseriesHistogram<double> timeseriesHistogram(
            1e3, -1e5, 1e5, MultiLevelTimeSeries<double>(100, { mDuration }));
        mBuckets.insert(pair<string, TimeseriesHistogram<double>>(name, timeseriesHistogram));
    }
    (mBuckets.find(name)->second).addValue(chrono::steady_clock::now(), value);
}

const std::string& PerformanceMarker::getLastReport()
{
    return getInstance().mReport;
}