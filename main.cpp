#include <iostream>
#include <random>
<<<<<<< HEAD
#include "src/integer.hpp"

int main()
{
    clampedBits left = "101011101011101011101011101011001011101011101011101011101011101011101011";
    clampedBits right = "101011101011101011101011101011101011101011101011101011101011101011101011";
    std::cout << "Left:\t" << left << '\n';
    std::cout << "Right:\t" << right << '\n';
    std::cout << std::boolalpha << (left < right) << '\n';
=======
#include "src/sieve.hpp"
#include <string>
int main()
{
    clampedBits left = "101111101101110111101111101101110111101111101101110111101010101010110101";
    std::cout << "Left:\t" << left << '\n';
    left >>= 67;
    std::cout << "Left:\t" << left << '\n';
    // "101111101101110111101111101101110111101111101101110111101010101010110101"
    // "000000000000000000000000000000000000000000000000000000000000000000010101"
>>>>>>> 03e042d (HUGE improvement)
}
