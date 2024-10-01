cmake_minimum_required(VERSION 3.28)
project(primesProject)

set(CMAKE_CXX_STANDARD 23)
# include_directories(/home/roxandi/repos/boost_1_86_0)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(primesProject main.cpp
        src/sieve.hpp
        src/clampedBits.hpp
)
