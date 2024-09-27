#include <iostream>
#include <fstream>
#include <iomanip>

#include "src/sieve.hpp"

int main()
{

    smallSieve sieve(15 * 1000 * 1000 * 10);
    std::cout << sieve.next(1546) << '\n';
}