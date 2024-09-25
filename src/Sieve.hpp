#pragma once
#ifndef SIEVE_HPP
#define SIEVE_HPP
#include "clampedBits.hpp"
#include <cmath>

class smallSieve
{
    clampedBits mData;
    uint64_t mLastChecked;
    void __sieve()
    {
        for (uint64_t idx = 3; idx * idx < std::sqrt(mData.size()); idx += 2)
        {
            if (mData.at(idx))
            {
                for (uint64_t j = idx * idx; j < mData.size(); j += 2 * idx)
                {
                    mData.set(j, 0);
                }
            }
        }
    }

public:
    explicit smallSieve(uint64_t n) : mData(clampedBits(n, 1)), mLastChecked(0)
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
        if (number >= mData.size())
            expand(number + 1);
        if (number != 0b10 && !(number & 1))
            return false;
        else
            return mData.at(number);
    }
    friend std::ostream &operator<<(std::ostream &os, smallSieve &sieve)
    {
        for (uint64_t idx = 2; idx < sieve.mData.size(); ++idx)
        {
            if (sieve.isPrime(idx))
                os << idx << ' ';
        }
        return os;
    }
};

#endif // !SIEVE_HPP
