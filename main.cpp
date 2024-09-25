#include <iostream>
#include <fstream>
#include <iomanip>

#include "src/Sieve.hpp"

int main()
{
    clampedBits left = 0b000111111, right = 0b1111;
    std::cout << (left != right);
}
