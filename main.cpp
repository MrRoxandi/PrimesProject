#include <iostream>
#include <random>
#include "src/integer.hpp"

int main()
{
    integer number(clampedBits("1010101010101010101010101010101010101010101"));
    integer number2(clampedBits("01010101010101010101010101010101010101010101"));
    std::cout << number2 << '\n';
}
