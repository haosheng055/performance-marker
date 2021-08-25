//
// Created by DELL on 2021/8/25.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>

#include "log/LogFile.h"
#include "log/Logger.h"

using namespace std;

std::unique_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

TEST(LogFileTest, bench)
{
    string fileName = "LogFileTest";
    g_logFile.reset(new LogFile(fileName, 200*1000, 10));
    Logger::setOutput(outputFunc);

    string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < 10000; ++i)
    {
        LOG_INFO << line << i;
        this_thread::sleep_for(100ms);
    }
}

