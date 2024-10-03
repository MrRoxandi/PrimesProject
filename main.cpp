#include "src/clampedBits.hpp"
#include <format>
#include <iostream>
#include <random>

int main() {
  clampedBits bits(1, 1);
  for (uint64_t i = 0; i < 128; i++) {
    bits <<= 2;
    if (i % 4 == 0) {
      std::cout << std::format("Bits:\t{}\n", bits.str());
      std::cout << std::format("Value:\t{}\n", bits.convert_to_base(10));
    }
  }

  return 0;
}
