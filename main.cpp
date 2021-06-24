#include <iostream>
#include <thread>

#include "src/BucketedTimeSeries.h"
#include "src/MultiLevelTimeSeries.h"
#include "src/TimeseriesHistogram.h"
#include "src/PerformanceMarker.h"

using namespace std;

int main()
{
    cout << "PerformanceMarker test:" << endl;
    PerformanceMarker::initialize("haosheng",5);

//    TODO:第一个被加入的value不会被丢弃？？
    std::this_thread::sleep_for(chrono::seconds(2));
    cout << "add value 100 int round 1\n";
    SOL2_PERFORMANCE_COUNT64("testInt64",100);

    std::this_thread::sleep_for(chrono::seconds(12));
    cout << "add value 10000 in round 3\n";
    SOL2_PERFORMANCE_COUNT64("testInt64",10000);

    std::this_thread::sleep_for(chrono::seconds(5));
    cout << "add value 20000 in round 4\n";
    SOL2_PERFORMANCE_COUNT64("testInt64",20000);

    std::this_thread::sleep_for(chrono::seconds(10));
    cout << "------------------------------------------" << endl;
}