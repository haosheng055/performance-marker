//
// Created by DELL on 2021/8/25.
//

#ifndef PERFORMANCEMARKER_ASYNCLOGGING_H
#define PERFORMANCEMARKER_ASYNCLOGGING_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include "LogFile.h"

// two buffers; func: append(), threadFuc()
// append(): when buffer A is full(?), notify the looping thread to log
// all the full buffers and the buffer being written.
// threadFunc(): log buffers, swap buffer B buffer A
class AsyncLogging {
public:
    AsyncLogging(std::string& fileName, int flushInterval);

    void append(const char* logLine, int len);

    void threadFunc();

    void stop()
    {
        mIsRunning = false;
        mCondition.notify_one();
        mThread.join();
    }

private:
    const int kFixedSize = 1000;
    const int kFlushInterval;
    std::string mFileName;
    std::atomic<bool> mIsRunning;
    std::thread mThread;

    using Buffer = std::stringstream;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;
    std::mutex mMutex;
    std::condition_variable mCondition;
    BufferPtr mCurrentBuffer;
    BufferVector mBuffers;
};

#endif // PERFORMANCEMARKER_ASYNCLOGGING_H
