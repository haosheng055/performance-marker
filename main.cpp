#include <iostream>
#include <thread>

#include "src/BucketedTimeSeries.h"
#include "src/MultiLevelTimeSeries.h"

using namespace std;

int main()
{
    std::chrono::steady_clock::duration cao[1];
    cout << "bucket test:" << endl;
    Bucket<double> bucket;
    bucket.addValue(1,14);
    cout << bucket.avg() << endl
         << bucket.mSum << endl << bucket.mCount << endl;
    cout << "------------------------------------------" << endl;

    cout << "BucketedTimeSeries test:" << endl;
    // cout << chrono::steady_clock::duration(1000000).count();
    BucketedTimeSeries<unsigned> bucketedTimeSeries(10,
        std::chrono::seconds(10));

    bucketedTimeSeries.addValue(chrono::steady_clock::now(),10000);
    std::this_thread::sleep_for(std::chrono::seconds (11));
    bucketedTimeSeries.addValue(chrono::steady_clock::now(),1);
    bucketedTimeSeries.addValue(chrono::steady_clock::now(),2);
    bucketedTimeSeries.addValue(chrono::steady_clock::now(),3);

    std::cout << "count: " << bucketedTimeSeries.count() << "\n"
        << "sum: " << bucketedTimeSeries.sum() << "\n"
        << "avg: " << bucketedTimeSeries.avg() <<"\n"
//        << "elapsed: " << bucketedTimeSeries.elapsed().count() << "\n";
        << "rate: " << bucketedTimeSeries.rate() << "\n"
        << "countRate: " << bucketedTimeSeries.countRate() << "\n";
    cout << "------------------------------------------" << endl;

    cout << "MultiLevelTimeSeries test:" << endl;
    MultiLevelTimeSeries<unsigned> multiLevelTimeSeries
        (10,{std::chrono::seconds(10),std::chrono::minutes(1)});

    multiLevelTimeSeries.addValue(chrono::steady_clock::now(),1000);
    std::this_thread::sleep_for(std::chrono::seconds (11));
    multiLevelTimeSeries.addValue(chrono::steady_clock::now(),1);
    multiLevelTimeSeries.addValue(chrono::steady_clock::now(),2);
    multiLevelTimeSeries.addValue(chrono::steady_clock::now(),3);

    multiLevelTimeSeries.update(chrono::steady_clock::now());
    std::cout << "time level is 10 seconds: " << std::endl;
    std::cout << "count: " << multiLevelTimeSeries.count(0) << "\n"
              << "sum: " << multiLevelTimeSeries.sum(0) << "\n"
              << "avg: " << multiLevelTimeSeries.avg(0) <<"\n"
              //        << "elapsed: " << bucketedTimeSeries.elapsed().count() << "\n";
              << "rate: " << multiLevelTimeSeries.rate(0) << "\n"
              << "countRate: " << multiLevelTimeSeries.countRate(0) << "\n";
    std::cout << "time level is 60 seconds: " << std::endl;
    std::cout << "count: " << multiLevelTimeSeries.count(1) << "\n"
              << "sum: " << multiLevelTimeSeries.sum(1) << "\n"
              << "avg: " << multiLevelTimeSeries.avg(1) <<"\n"
              //        << "elapsed: " << bucketedTimeSeries.elapsed().count() << "\n";
              << "rate: " << multiLevelTimeSeries.rate(1) << "\n"
              << "countRate: " << multiLevelTimeSeries.countRate(1) << "\n";
}