cmake_minimum_required(VERSION 3.29)
project(primesProject)

set(CMAKE_CXX_STANDARD 26)

add_executable(primesProject main.cpp
        src/Sieve.hpp
        src/bitsarray.hpp
        src/RadInt.hpp

)
