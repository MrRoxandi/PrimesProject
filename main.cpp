#include <iostream>
#include <random>
#include "src/integer.hpp"

int main()
{
    clampedBits left = "101011101011101011101011101011001011101011101011101011101011101011101011";
    clampedBits right = "101011101011101011101011101011101011101011101011101011101011101011101011";
    std::cout << "Left:\t" << left << '\n';
    std::cout << "Right:\t" << right << '\n';
    std::cout << std::boolalpha << (left < right) << '\n';
}
