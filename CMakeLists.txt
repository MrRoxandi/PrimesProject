cmake_minimum_required(VERSION 3.29)
project(primesProject)

set(CMAKE_CXX_STANDARD 23)
include_directories(C:/boost_1_86_0/)

add_executable(primesProject main.cpp
        src/sieve.hpp
        src/clampedBits.hpp
        src/RadInt.hpp
)
