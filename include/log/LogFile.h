//
// Created by DELL on 2021/8/24.
//

#ifndef PERFORMANCEMARKER_LOGFILE_H
#define PERFORMANCEMARKER_LOGFILE_H

#include <fstream>
#include <iostream>
#include <string>

#include "TimeStamp.h"

class LogFile {
public:
    LogFile(std::string& fileName, uint32_t rollSize, uint32_t flushInterval);
    ~LogFile() = default;

    void append(const char* logLine, int len);

    void rollFile();

    void flush() { mFile.flush(); }

    static std::string getLogFileName(const std::string& fileName);

private:
    std::string mFileName;
    std::ofstream mFile;

    uint32_t mWrittenBytes;
    uint32_t mRollSize;
    uint32_t mFlushInterval;
    uint32_t mLastFlush;
};

#endif // PERFORMANCEMARKER_LOGFILE_H
