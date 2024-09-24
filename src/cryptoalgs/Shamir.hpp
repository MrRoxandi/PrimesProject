#pragma once

#ifndef SHAMIR_HPP
#define SHAMIR_HPP

#include "../src/RadInt.hpp"

class Shamir {
	cpp_int c, d;
public:
	Shamir(const cpp_int& p) {
		this->c = primes::getRandomPrime(2, p - 1);
		this->d = functions::invMod(c, p - 1);
	}
	const cpp_int& getC() const { return c; }
	const cpp_int& getD() const { return d; }
};

#endif // !SHAMIR_HPP
