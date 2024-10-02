#ifndef INTEGER_HPP
#define INTEGER_HPP
#include "clampedBits.hpp"
#include <cstdint>
#include <ostream>

class integer
{
    clampedBits mData;
    bool mLowerZero;

    static integer sum_of_positiv(const integer &first, const integer &second)
    {
        uint64_t max_size = std::max(first.mData.size(), second.mData.size()) + 1;
        clampedBits result(max_size, 0);
        uint64_t carry = 0;
        for (uint64_t idx = 0; idx < max_size; ++idx)
        {
            uint64_t current_bit = first.mData[idx] + second.mData[idx] + carry;
            result.set(idx, current_bit % 2);
            carry = current_bit / 2;
        }
        result.trim();
        return {result};
    }

public:
    integer(const clampedBits &bits) : mData(bits), mLowerZero(0) {}
    integer(const int64_t number) : mData(std::abs(number)), mLowerZero(number < 0) {}
    integer(const integer &) = default;
    integer(integer &&) = default;
    integer &operator=(const integer &) = default;
    integer &operator=(integer &&) = default;
    friend integer operator+(const integer &lhs, const integer &rhs)
    {
        return integer::sum_of_positiv(lhs, rhs);
    }
    friend std::ostream &operator<<(std::ostream &os, const integer &it)
    {
        if (it.mLowerZero)
            os << '-';
        os << it.mData.base_str(10);
        return os;
    }
};

#endif
