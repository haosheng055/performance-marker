//
// Created by DELL on 2021/8/25.
//
#include "log/AsyncLogging.h"
#include <cassert>

using namespace std;

AsyncLogging::AsyncLogging(string& fileName, int flushInterval)
    : kFlushInterval(flushInterval)
    , mFileName(fileName)
    , mIsRunning(true)
    , mThread([this]() { threadFunc(); })
    , mMutex()
    , mCondition()
    , mCurrentBuffer(new Buffer)
    , mBuffers()
{
    mBuffers.reserve(16);
}

void AsyncLogging::append(const char* logLine, int len)
{
    unique_lock<mutex> uniqueLock(mMutex);
    if (mCurrentBuffer->str().size() + len < kFixedSize) {
        (*mCurrentBuffer) << logLine;
    } else {
        mBuffers.push_back(move(mCurrentBuffer));
        mCurrentBuffer.reset(new Buffer);
        (*mCurrentBuffer) << logLine;
        mCondition.notify_one();
    }
}

void AsyncLogging::threadFunc()
{
    auto newBuffer = make_unique<Buffer>();
    BufferVector bufferVector;
    bufferVector.reserve(16);
    LogFile logFile(mFileName, kFixedSize * 1000, kFlushInterval);
    while (mIsRunning) {
        assert(bufferVector.empty());
        {
            unique_lock<mutex> uniqueLock(mMutex);
            if (mBuffers.empty()) {
                mCondition.wait_for(uniqueLock, chrono::seconds(kFlushInterval));
            }
            mBuffers.push_back(move(mCurrentBuffer));
            mCurrentBuffer = move(newBuffer);
            bufferVector.swap(mBuffers);
        }
        assert(!bufferVector.empty());

        for (auto& buffer : bufferVector) {
            logFile.append(buffer->str().c_str(), buffer->str().size());
        }
        logFile.flush();

        if (!newBuffer) {
            newBuffer = move(bufferVector.back());
            bufferVector.pop_back();
            newBuffer->clear();
        }
        bufferVector.clear();
    }
    logFile.flush();
}