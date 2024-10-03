#ifndef INTEGER_HPP
#define INTEGER_HPP
#include "clampedBits.hpp"
#include <cstdint>
#include <ostream>

class integer
{
    clampedBits mData;
    bool mLowerZero;

public:
    integer(const clampedBits &bits) : mData(bits), mLowerZero(0) {}
    integer(const int64_t number) : mData(std::abs(number)), mLowerZero(number < 0) {}
    integer(const integer &) = default;
    integer(integer &&) = default;
    integer &operator=(const integer &) = default;
    integer &operator=(integer &&) = default;
};

#endif
