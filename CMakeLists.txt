cmake_minimum_required(VERSION 3.19)
project(performance)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES
        src/Bucket.h
        src/BucketedTimeSeries.h
        src/MultiLevelTimeSeries.h
    )

add_executable(performance main.cpp ${SOURCES})