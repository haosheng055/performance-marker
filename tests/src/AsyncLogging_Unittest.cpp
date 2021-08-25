//
// Created by DELL on 2021/8/25.
//
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

#include <log/AsyncLogging.h>
#include <log/Logger.h>

using namespace std;

AsyncLogging* g_asyncLog = nullptr;

void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void bench()
{
    Logger::setOutput(asyncOutput);

    string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < 10000; ++i)
    {
        LOG_INFO << line << i;
        this_thread::sleep_for(100ms);
    }
}

TEST(AsyncLoggingTest, bench)
{
    printf("pid = %d\n", getpid());

    string fileName = "AsyncLoggingTest";
    AsyncLogging log(fileName, 10);
    g_asyncLog = &log;

    bench();
}
