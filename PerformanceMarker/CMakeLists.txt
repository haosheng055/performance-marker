add_executable(PerformanceMarker main.cpp)

target_link_libraries(PerformanceMarker
        $<TARGET_NAME:PerformanceMarkerApi>
        )
set_target_properties(PerformanceMarker
        PROPERTIES
        CXX_STANDARD 17
        )