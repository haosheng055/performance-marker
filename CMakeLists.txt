cmake_minimum_required(VERSION 3.19)
project(performance)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES
        src/BucketedTimeSeries.cpp
        src/MultiLevelTimeSeries.cpp
        src/Histogram.cpp)

add_executable(performance main.cpp ${SOURCES})