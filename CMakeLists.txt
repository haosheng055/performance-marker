cmake_minimum_required(VERSION 3.19)
project(performance)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES
        include/Bucket.h
        include/BucketedTimeSeries.h
        include/MultiLevelTimeSeries.h
        include/HistogramBuckets.h
        include/TimeseriesHistogram.h
        src/PerformanceMarker.cpp
        include/Defer.h)

add_executable(performance main.cpp ${SOURCES})