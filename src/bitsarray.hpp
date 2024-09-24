#pragma once
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <type_traits>
#include <vector>

#include <ostream>
#include <concepts>


class bitsarray {
    uint64_t mSize;
    std::vector<uint64_t> mBits;
public:
    static constexpr uint64_t ONES = std::numeric_limits<uint64_t>::max();
    bitsarray(const uint64_t size, const unsigned int filler = 0) : mSize(size) {
        mBits.resize((size + 63) / 64, filler ? ONES : 0);
    }

    bitsarray(const bitsarray& other) : mSize(other.mSize), mBits(other.mBits) {}

    bitsarray(bitsarray&& other) noexcept : mSize(other.mSize), mBits(std::move(other.mBits)) {
        other.mSize = 0;
    }

    bitsarray& operator=(const bitsarray& other) {
        if (this == &other) return *this;
        mSize = other.mSize;
        mBits = other.mBits;
        return *this;
    }

    bitsarray& operator=(bitsarray&& other) noexcept {
        if (this == &other) return *this;
        mSize = other.mSize;
        mBits = std::move(other.mBits);
        other.mSize = 0;
        return *this;
    }

    void expand(const uint64_t new_size, const unsigned int filler = 0) {
        if (new_size <= mSize) return;
        uint64_t old_size_blocks = (mSize + 63) / 64;
        uint64_t new_size_blocks = (new_size + 63) / 64;
        if (new_size_blocks > old_size_blocks) {
            mBits.resize(new_size_blocks, filler ? ONES : 0);
        }
        mSize = new_size;
    }
    unsigned int operator[](const uint64_t pos) const {
        if (pos >= mSize) throw std::out_of_range("Position out of range");
        uint64_t block_p = pos / 64, bit_p = pos % 64;
        return (mBits[block_p] >> bit_p) & 1;
    }
    [[nodiscard]] unsigned int get(uint64_t position) const {
        if (position >= mSize) throw std::out_of_range("Position out of range");
        auto block_p = position / 64, bit_p = position % 64;
        return (mBits[block_p] >> bit_p) & 1;
    }

    void set(uint64_t position, unsigned int val) {
        if (position >= mSize) throw std::out_of_range("Position out of range");
        auto block_p = position / 64, bit_p = position % 64;
        if (val) {
            mBits[block_p] |= (1ull << bit_p);
        }
        else {
            mBits[block_p] &= ~(1ull << bit_p);
        }
    }
    
    bool operator==(const bitsarray& other) const {
        return mSize == other.mSize && mBits == other.mBits;
    }

    bool operator!=(const bitsarray& other) const {
        return !(*this == other);
    }
    friend std::ostream& operator<<(std::ostream& os, const bitsarray& ba) {
        for (uint64_t idx = 0; idx < ba.mSize; ++idx) {
            os << (ba.get(idx) ? '1' : '0');
        }
        return os;
    }
};