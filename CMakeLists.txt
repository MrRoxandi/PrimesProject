cmake_minimum_required(VERSION 3.28)
project(primesProject)

set(CMAKE_CXX_STANDARD 26)
include_directories(C:/boost_1_86_0/)

add_compile_options(-Wall -Wextra -Werror -m64)

add_executable(primesProject main.cpp
        src/sieve.hpp
        src/clampedBits.hpp
        src/integer.hpp
)
