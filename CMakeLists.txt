cmake_minimum_required(VERSION 3.13)  # CMake version check
project(inline_format)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++14 standard



add_executable(infmt_test test.cpp)
add_executable(example example.cpp)

# add_executable(bench benchmark.cpp)
# target_link_libraries(bench benchmark pthread)
