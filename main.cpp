#include <iostream>
#include <fstream>
#include <iomanip>

#include "src/Sieve.hpp"

int main() {
    std::ofstream file(R"(F:/Repos/primesProject/primes.txt)");
    auto start = clock();
    smallSieve sieve(3600000000);
    auto stop = clock();
    file << sieve << std::endl;
    file << "Time taken: " << std::setprecision(5) << static_cast<double>(stop - start) / CLOCKS_PER_SEC << " s" << std::endl;

}
