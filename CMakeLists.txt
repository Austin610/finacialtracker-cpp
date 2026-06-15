cmake_minimum_required(VERSION 3.14)

project(FinancialTracker VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#  Library (shared between main program and tests) 
add_library(tracker_lib STATIC
    src/FinancialTracker.cpp
)

target_include_directories(tracker_lib PUBLIC
    include
)


#  Main executable 
add_executable(financial_tracker
    src/main.cpp
)

target_link_libraries(financial_tracker PRIVATE
    tracker_lib
)


#  Tests 
enable_testing()

add_executable(tracker_tests
    tests/test_tracker.cpp
)

target_include_directories(tracker_tests PRIVATE
    include
    tests
)

target_link_libraries(tracker_tests PRIVATE
    tracker_lib
)

add_test(
    NAME FinancialTrackerTests
    COMMAND tracker_tests
)