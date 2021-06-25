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
    {
        SOL2_PERFORMANCE_MEASURE("ezereal");
        this_thread::sleep_for(chrono::seconds(2));
    }
    this_thread::sleep_for(chrono::seconds(5));
    cout << "------------------------------------------" << endl;
}