//
// Created by DELL on 2021/8/24.
//

#include "log/LogFile.h"
#include <chrono>
#include <ctime>
#include <thread>

using namespace std;

LogFile::LogFile(std::string& fileName, uint32_t rollSize = 10*1000, uint32_t flushInterval = 10)
    : mFileName(fileName)
    , mFile()
    , mWrittenBytes(0)
    , mRollSize(rollSize)
    , mFlushInterval(flushInterval)
    , mLastFlush(0)
{
    mFile.open(getLogFileName(fileName), ofstream::app);
}

void LogFile::append(const char* logLine, int len)
{
    if (!(mFile << logLine)) {
        std::cerr << "AppendFile::append() failed\n";
        return;
    }
    mWrittenBytes += len;
    if (mWrittenBytes > mRollSize) {
        rollFile();
    } else {
        auto now = chrono::duration_cast<chrono::seconds>(
            chrono::system_clock::now().time_since_epoch())
                       .count();
        if (now >= mLastFlush + mFlushInterval) {
            mLastFlush = now;
            mFile.flush();
        }
    }
}

void LogFile::rollFile()
{
    string fileName = getLogFileName(mFileName);
    if (fileName != mFileName) {
        mFile.close();
        mFileName = fileName;
        mFile.open(fileName, ofstream::app);
        mWrittenBytes = 0;
    }
}

string LogFile::getLogFileName(const string& fileName)
{
    string filename;
    filename.reserve(fileName.size() + 64);
    filename = fileName;

    char timebuf[32];
    struct tm tm;
    time_t seconds = static_cast<time_t>(chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count());
    localtime_s(&tm, &seconds);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, "%d", this_thread::get_id());
    filename += pidbuf;

    filename += ".log";

    return filename;
}
