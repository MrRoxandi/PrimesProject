#pragma once
#ifndef SIEVE_HPP
#define SIEVE_HPP
#include "bitsarray.hpp"


class smallSieve {
    bitsarray mData;
    uint64_t mSize;

    void __sieve() {
        // Sieve of Eratosthenes
        for (uint64_t idx = 3; idx * idx < mSize; idx += 2) {
            if (mData.get(idx)) {
                for (uint64_t j = idx * idx; j < mSize; j += 2 * idx) {
                    mData.set(j, 0);
                }
            }
        }
    }

public:
    explicit smallSieve(uint64_t n) : mData(bitsarray(n, 1)), mSize(n) {
        __sieve();
    }
    void expand(uint64_t n) {
        if (n <= mSize) return;
        mData.expand(n, 1);
        mSize = n;
        __sieve();
    }
    [[nodiscard]] uint64_t prev(uint64_t number) const {
        if (number >= mSize) throw std::out_of_range("Number out of range");
        for (auto idx = number - 1; idx >= 2; --idx) {
            if (mData.get(idx)) return idx;
        }
        return 1;
    }
    [[nodiscard]] uint64_t next(uint64_t number) {
        if (number >= mSize) expand(std::max(mSize * 2, number * 2));
        for (auto idx = number + 1; idx < mSize; ++idx) {
            if (mData.get(idx)) return idx;
        }
        return 0;
    }
    [[nodiscard]] bool isPrime(uint64_t number) {
        if (number >= mSize) expand(number + 1);
        if(number != 0b10 && !(number & 1)) return false;
        else return mData.get(number);
    }
    [[nodiscard]] std::vector<uint64_t> getPrimes() const {
        std::vector<uint64_t> primes;
        for (uint64_t idx = 2; idx < mSize; ++idx) {
            if (mData.get(idx)) {
                primes.push_back(idx);
            }
        }
        return primes;
    }

    friend std::ostream& operator<<(std::ostream& os, smallSieve& sieve) {
        for (uint64_t idx = 2; idx < sieve.mSize; ++idx) {
            if (sieve.isPrime(idx)) os << idx << ' ';
        }
        return os;
    }
};

#endif // !SIEVE_HPP
