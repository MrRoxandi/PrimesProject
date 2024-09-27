#include <iostream>
#include <fstream>
#include <iomanip>

#include "src/sieve.hpp"

int main()
{

    clampedBits first = 0b1110101;
    clampedBits second = 0b0001010;
    second &= ~first;
    std::cout << (second ^ second) << '\n';
}