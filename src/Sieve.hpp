#pragma once
#ifndef SIEVE_HPP
#define SIEVE_HPP
#include "clampedBits.hpp"
#include <cmath>

class smallSieve
{
    clampedBits mData;
    void __sieve()
    {
        for (uint64_t possible_prime = 3; possible_prime * possible_prime < mData.size(); possible_prime += 2)
            if (mData.at(possible_prime))
                for (uint64_t not_prime = possible_prime * possible_prime; not_prime < mData.size(); not_prime += 2 * possible_prime)
                    mData.set(not_prime, 0);
    }

public:
    explicit smallSieve(uint64_t n) : mData(clampedBits(n, 1))
    {
        mData.set(0, 0);
        mData.set(1, 0);
        __sieve();
    }

    [[nodiscard]] uint64_t size() const { return mData.size(); }

    void expand(uint64_t n)
    {
        if (n <= mData.size())
            return;
        mData.expand(n, 1);
        __sieve();
    }

    [[nodiscard]] uint64_t prev(uint64_t number) const
    {
        if (number >= mData.size())
            throw std::out_of_range("Given number is too large for this sieve");
        uint64_t start, step = 2;
        for (auto idx = number - (!(number & 1) ? 1 : 2); idx >= 2; idx -= 2)
        {
            if (mData.at(idx))
                return idx;
        }
        return 1;
    }
    [[nodiscard]] uint64_t next(uint64_t number)
    {
        if (number >= mData.size())
            throw std::runtime_error("Given number is too large for this sieve");
        for (auto idx = number + (!(number & 1) ? 1 : 2); idx < mData.size(); idx += 2)
        {
            if (mData.at(idx))
                return idx;
        }
        return 0;
    }
    [[nodiscard]] bool isPrime(uint64_t number)
    {
        if (number <= 1)
            return false;
        if (number <= 3)
            return true;
        if (!(number & 1))
            return false;
        return mData.at(number);
    }
    friend std::ostream &operator<<(std::ostream &os, smallSieve &sieve)
    {
        os << 2 << ' ';
        for (uint64_t prime = 3; prime < sieve.mData.size(); prime += 2)
        {
            if (sieve.isPrime(prime))
                os << prime << ' ';
        }
        return os;
    }
};

#endif // !SIEVE_HPP
