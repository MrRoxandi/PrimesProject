#pragma once
#ifndef _RSA_ALG_HPP
#define _RSA_ALG_HPP

#include "../RadInt.hpp"

class RSA {
private:
  cpp_int _n = 0, _d = 0, _c = 0;

public:
  static std::array<cpp_int, 2> createPair(const size_t bit_count) {
    cpp_int first, second;
    first = primes::getRandomPrime(bit_count);
    do {
      second = primes::getRandomPrime(bit_count);
    } while (first == second);
    return std::array{first, second};
  }
  static std::array<cpp_int, 2> createPair(const cpp_int &lower_bound,
                                           const cpp_int &upper_bound) {
    cpp_int first, second;
    first = primes::getRandomPrime(lower_bound, upper_bound);
    do {
      second = primes::getRandomPrime(lower_bound, upper_bound);
    } while (first == second);
    return std::array{first, second};
  }

  static std::array<cpp_int, 2> createInversePair(const cpp_int &modulus) {
    cpp_int first, second;
    do {
      first = integers::getRandomInteger(1, modulus - 1);
    } while (functions::gcd(first, modulus) != 1);
    second = functions::invMod(first, modulus);
    return std::array{first, second};
  }

  RSA() {}
  RSA(const cpp_int &P, const cpp_int &Q) {
    cpp_int fi = (P - 1) * (Q - 1);
    _n = P * Q;
    std::array<cpp_int, 2> inv_pair = createInversePair(fi);
    _d = inv_pair[0];
    _c = inv_pair[1];
  }
  cpp_int getKey(const uint8_t &n = 1) const { return (n == 0) ? _d : _c; }
  cpp_int getN() const { return _n; }
};

#endif
