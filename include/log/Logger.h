//
// Created by DELL on 2021/8/24.
//

#ifndef PERFORMANCEMARKER_LOGGER_H
#define PERFORMANCEMARKER_LOGGER_H

#include <chrono>
#include <sstream>
#include <string>

#include "log/TimeStamp.h"

class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FALTAL
    };

    class BaseFileName {
    public:
        template <int N>
        BaseFileName(const char (&fileName)[N])
            : mFileName(fileName)
            , mSize(N - 1)
        {
            auto result = strrchr(mFileName, '\\');
            if (result) {
                mFileName = result + 1;
                mSize -= (mFileName - fileName);
            }
        }
        explicit BaseFileName(const char* fileName)
            : mFileName(fileName)
        {
            auto result = strrchr(fileName, '\\');
            if (result) {
                mFileName = result + 1;
            }
            mSize = static_cast<uint32_t>(strlen(mFileName));
        }
        const char* getFileName() const
        {
            return mFileName;
        }

    private:
        const char* mFileName; // non-path
        uint32_t mSize;
    };

    Logger(BaseFileName file, int line);
    Logger(BaseFileName file, int line, LogLevel level);
    ~Logger();

    static LogLevel getLogLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    static void setOutput(OutputFunc outputFunc);

    std::stringstream& getStream() { return mImpl.mStream; }

private:
    class Impl {
    public:
        Impl(LogLevel level, const BaseFileName& file, int line);
        void addSourceFile();
        Timestamp mTimestamp;
        std::stringstream mStream;
        LogLevel mLevel;
        int mLine;
        BaseFileName mBaseFileName;
    };
    Impl mImpl;
};

#define LOG_DEBUG Logger(__FILE__, __LINE__, Logger::DEBUG).getStream()
#define LOG_INFO Logger(__FILE__, __LINE__).getStream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).getStream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).getStream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).getStream()

#endif // PERFORMANCEMARKER_LOGGER_H
