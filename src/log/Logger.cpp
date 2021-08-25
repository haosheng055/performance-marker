//
// Created by DELL on 2021/8/24.
//

#include <thread>

#include "log/Logger.h"

const char* LogLevelName[] = {
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL "
};

void defaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::LogLevel g_logLevel = Logger::INFO;

Logger::Impl::Impl(Logger::LogLevel level, const Logger::BaseFileName& file, int line)
: mLevel(level)
, mTimestamp(Timestamp::now())
, mStream()
, mBaseFileName(file)
, mLine(line)
{
    mStream << mTimestamp.toFixedSizeString() << ' ';
    mStream << LogLevelName[mLevel];
    mStream << std::this_thread::get_id() << ' ';
}

void Logger::Impl::addSourceFile()
{
    mStream << " - " << mBaseFileName.getFileName() << ':' << mLine << '\n';
}

Logger::Logger(Logger::BaseFileName file, int line)
    : mImpl(Logger::INFO, file, line)
{
}

Logger::Logger(Logger::BaseFileName file, int line, Logger::LogLevel level)
    : mImpl(level, file, line)
{
}

Logger::~Logger()
{
    mImpl.addSourceFile();
    auto str = mImpl.mStream.str();
    g_output(str.c_str(), str.size());
}

Logger::LogLevel Logger::getLogLevel()
{
    return g_logLevel;
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(Logger::OutputFunc outputFunc)
{
    g_output = outputFunc;
}
