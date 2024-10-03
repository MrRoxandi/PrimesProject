#include <iostream>
#include <random>
#include "src/clampedBits.hpp"

int main()
{
    clampedBits left = "100010101";
    left = left + 1;
    std::cout << left << '\n';
    return 0;
}
