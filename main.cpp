#include <iostream>
#include <fstream>
#include <iomanip>

#include "src/sieve.hpp"

int main()
{

    std::ofstream file(R"(F:\Repos\primesProject\primes.txt)");
    smallSieve sieve(36ull * 1000ull * 1000ull * 100ull);
    file << sieve << std::endl;
    file.close();
}