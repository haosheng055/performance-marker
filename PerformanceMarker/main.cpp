#include <iostream>
#include <thread>

#include "BucketedTimeSeries.h"
#include "MultiLevelTimeSeries.h"
#include "PerformanceMarker.h"
#include "TimeseriesHistogram.h"
#include "HistogramBuckets.h"

using namespace std;

int main()
{
    cout << "PerformanceMarker test:" << endl;
    PerformanceMarker::initialize("haosheng", 5);
    {
        SOL2_PERFORMANCE_MEASURE("ezereal");
        this_thread::sleep_for(chrono::seconds(2));
    }
    this_thread::sleep_for(chrono::seconds(5));
    cout << "------------------------------------------" << endl;
}