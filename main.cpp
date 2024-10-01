#include <iostream>
#include <random>
#include "src/sieve.hpp"

int main()
{
    const uint64_t size = 25;
    std::mt19937_64 generator(std::random_device{}());
    std::uniform_int_distribution<unsigned int> dist(0, 1);
    clampedBits left(size, 0), right(size, 1);
    for (uint64_t idx = 0; idx < size; ++idx)
    {
        left.set(idx, dist(generator));
        right.set(idx, dist(generator));
    }
    std::cout << "Left:\t" << left << '\n';
    std::cout << "Right:\t" << right << '\n';
    std::cout << "Result:\t" << (left ^ right) << '\n';
}
