#include <iostream>
#include <thread>

#include "PerformanceMarker.h"

using namespace std;

int main()
{
    cout << "PerformanceMarker test:" << endl;
    PerformanceMarker::initialize("haosheng", 5);
    {
        SOL2_PERFORMANCE_MEASURE("ezereal");
        this_thread::sleep_for(chrono::seconds(2));
    }
    auto i = 1;
    this_thread::sleep_for(chrono::seconds(5));
    cout << "------------------------------------------" << endl;

    {
        LOG_ERROR << "haosheng"
                  << "xininazi";
        LOG_INFO << "haosheng"
                 << "xininazi";
        LOG_DEBUG << "fslfjs";
        LOG_WARN << "warn!!!!!!!";
        string filename = "haosen";
        LogFile logFile { filename, 10, 1 };
        logFile.append("12345678911", 11);
        std::this_thread::sleep_for(1500ms);
        logFile.append("464654", 6);
        std::this_thread::sleep_for(1500ms);
        logFile.append("haosheng", 8);
        std::this_thread::sleep_for(20s);
    }
}