#pragma once
#ifndef RANDOM_INTEGER_H
#define RANDOM_INTEGER_H 1.1

#include "boost/multiprecision/cpp_int.hpp"
#include "boost/multiprecision/miller_rabin.hpp"
#include "boost/random.hpp"
#include <array>

using boost::multiprecision::cpp_int;
using std::size_t;

constexpr auto testIterations = 20;

namespace functions {
cpp_int mulMod(cpp_int a, cpp_int b, cpp_int m) {
  cpp_int result{0};
  for (; a != 0; a >>= 1) {
    if (a & 1)
      result = (result + b) % m;
    b = (b << 1) % m;
  }
  return result;
}

cpp_int powMod(cpp_int a, cpp_int b, cpp_int m) {
  cpp_int result = 1;
  a %= m;
  for (; b > 0; b >>= 1) {
    if (b & 1)
      result = mulMod(result, a, m);
    a = mulMod(a, a, m);
  }
  return result % m;
}

bool isPrime(cpp_int number) {
  return boost::multiprecision::miller_rabin_test(number, testIterations);
}

size_t bitCount(cpp_int number) {
  return static_cast<size_t>(std::ceil(boost::integer_log2(number)));
}

cpp_int gcd(cpp_int a, cpp_int b) {
  cpp_int first = a, second = b, tmp = 0;
  while (second != 0) {
    tmp = first % second;
    first = second;
    second = tmp;
  }
  return first;
}

struct cpp_int3D_t {
  cpp_int first, second, third;
};

cpp_int3D_t Euclid_alg(cpp_int a, cpp_int b) {
  cpp_int x = 0, y = 1, u = 1, v = 0;
  cpp_int q, m, n, r;
  while (a != 0) {
    q = b / a;
    r = b % a;
    m = x - u * q;
    n = y - v * q;
    b = a;
    a = r;
    x = u;
    y = v;
    u = m;
    v = n;
  }
  return {b, x, y};
}

cpp_int invMod(cpp_int a, cpp_int p) {
  if (a <= 0 || p <= 0) {
    throw std::invalid_argument("Both a and p must be positive numbers");
  }
  cpp_int3D_t res = Euclid_alg(a, p);
  if (res.first != 1) {
    throw std::runtime_error("Modular inverse does not exist");
  }
  cpp_int inverse = res.second;
  if (inverse < 0) {
    inverse += p;
  }
  return inverse;
}

cpp_int fromHex(const std::string_view str) {
  cpp_int result = 0;
  for (const auto &item : str) {
    if (!std::isxdigit(item))
      throw std::runtime_error("Given string isn't hex to dec convertable");
    result <<= 4;
    if (std::isdigit(item))
      result += static_cast<int>(item - '0');
    else
      result += static_cast<int>(std::toupper(item) - 'A' + 10);
  }
  return result;
}
} // namespace functions

namespace integers {
cpp_int getRandomBits(const size_t bit_count) {
  boost::random::mt19937_64 genEngine{std::random_device{}()};
  boost::random::uniform_int_distribution<int> intDistribution(0, 1);
  cpp_int result{1};
  for (size_t i = 0; i < bit_count - 1; ++i) {
    result <<= 1;
    result |= intDistribution(genEngine);
  }
  return result;
}

cpp_int getRandomInteger(size_t bit_count) { return getRandomBits(bit_count); }

cpp_int getRandomInteger(cpp_int lower_bound, cpp_int upper_bound) {
  boost::random::uniform_int_distribution<cpp_int> intDistribution(lower_bound,
                                                                   upper_bound);
  boost::random::mt19937_64 genEngine(std::random_device{}());
  return intDistribution(genEngine);
}
}; // namespace integers

namespace primes {

/*
Some first primes lower then 400
We using them to fast check is it prime cpp_int or not
*/
constexpr std::array first_primes = {
    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,
    43,  47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97,  101,
    103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167,
    173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239,
    241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
    317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397};

cpp_int getLowLevelPrime(const size_t bit_count) {
  cpp_int candidate;
  while (true) {
    candidate = integers::getRandomBits(bit_count) | cpp_int{1};
    bool is_prime = true;
    for (const auto &item : first_primes) {
      if (candidate == item)
        return candidate;
      else if (candidate % item == 0) {
        is_prime = false;
        break;
      }
    }
    if (is_prime)
      return candidate;
  }
}

cpp_int getLowLevelPrime(const cpp_int &lower_bound,
                         const cpp_int &upper_bound) {
  cpp_int candidate;
  while (true) {
    candidate = integers::getRandomInteger(lower_bound, upper_bound) | 1;
    bool is_prime = true;
    for (const auto &item : first_primes) {
      if (candidate == item)
        return candidate;
      else if (candidate % item == 0) {
        is_prime = false;
        break;
      }
    }
    if (is_prime)
      return candidate;
  }
}

cpp_int getRandomPrime(const size_t bit_count) {
  cpp_int candidate;
  while (true) {
    candidate = getLowLevelPrime(bit_count);
    if (functions::isPrime(candidate))
      return candidate;
  }
}

cpp_int getRandomPrime(const cpp_int &lower_bound, const cpp_int &upper_bound) {
  cpp_int candidate;
  while (true) {
    candidate = getLowLevelPrime(lower_bound, upper_bound);
    if (functions::isPrime(candidate))
      return candidate;
  }
}
} // namespace primes

#endif // !RANDOM_INTEGER_H
