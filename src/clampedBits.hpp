#pragma once
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <memory>
#include <ostream>
#include <string_view>
#include <string>
#include <algorithm>
#include <cstring>
#include <cmath>

class clampedBits
{
    std::unique_ptr<uint64_t[]> mData;
    uint64_t mSize, mBlocks;
    static uint64_t bitlen(uint64_t i)
    {
        return (i > 0) ? static_cast<uint64_t>(std::ceil(std::log2(i))) : 1;
    }

public:
    static constexpr uint64_t ONES = std::numeric_limits<uint64_t>::max();

    clampedBits(const uint64_t bit_count, unsigned int filler)
    {
        mSize = bit_count;
        mBlocks = bit_count / 64 + 1;
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::fill_n(mData.get(), mBlocks, (filler) ? ONES : 0);
    }

    clampedBits(const char *binary_string) : mSize(std::strlen(binary_string)), mBlocks(std::strlen(binary_string) / 64 + 1)
    {
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::fill_n(mData.get(), mBlocks, 0);
        for (uint64_t idx = 0; idx < mSize; ++idx)
        {
            if (auto bit = static_cast<uint64_t>(binary_string[idx] - '0'); bit > 1)
                throw std::runtime_error("Invalid binary string");
            else
                mData.get()[idx / 64] |= bit << (idx % 64);
        }
    }

    clampedBits(const uint64_t bitsData) : mSize(clampedBits::bitlen(bitsData)), mBlocks(1), mData(new uint64_t[1]{bitsData}) {}

    clampedBits(const std::string_view binary_string) : mSize(binary_string.length()), mBlocks(binary_string.length() / 64 + 1)
    {
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::fill_n(mData.get(), mBlocks, 0);
        for (uint64_t idx = 0; idx < mSize; ++idx)
        {
            if (auto bit = static_cast<uint64_t>(binary_string[idx] - '0'); bit > 1)
                throw std::runtime_error("Invalid binary string");
            else
                mData.get()[idx / 64] |= bit << (idx % 64);
        }
    }
    clampedBits(const clampedBits &other)
    {
        mSize = other.mSize;
        mBlocks = other.mBlocks;
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::copy_n(other.mData.get(), mBlocks, mData.get());
    }
    clampedBits(clampedBits &&) noexcept = default;

    clampedBits &operator=(const clampedBits &other)
    {
        mSize = other.mSize;
        mBlocks = other.mBlocks;
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::copy_n(other.mData.get(), mBlocks, mData.get());
        return *this;
    }
    clampedBits &operator=(clampedBits &&) noexcept = default;

    friend bool operator==(const clampedBits &lhs, uint64_t rhs)
    {
        return lhs == clampedBits(rhs);
    }
    friend bool operator!=(const clampedBits &lhs, uint64_t rhs)
    {
        return !(lhs == rhs);
    }
    friend bool operator==(const clampedBits &lhs, const clampedBits &rhs)
    {
        auto temp = lhs ^ rhs;
        for (uint64_t idx = 0; idx < temp.mBlocks; ++idx)
            if (temp.mData[idx] & 1)
                return false;
        return true;
    }

    friend bool operator!=(const clampedBits &lhs, const clampedBits &rhs)
    {
        return !(lhs == rhs);
    }

    unsigned int operator[](const uint64_t position) const
    {
        if (position >= mSize)
            throw std::out_of_range("");
        return (mData.get()[position / 64] >> (position % 64)) & 1;
    }

    clampedBits operator|(const clampedBits &other) const
    {
        clampedBits temp(std::max(mBlocks, other.mBlocks), 0);
        for (uint64_t block_idx = 0; block_idx < temp.mBlocks; ++block_idx)
        {
            auto first = 0ull, second = 0ull;
            if (block_idx < mBlocks)
                first = mData.get()[block_idx];
            if (block_idx < other.mBlocks)
                second = other.mData.get()[block_idx];
            temp.mData.get()[block_idx] = first | second;
        }
        return temp;
    }
    void operator|=(const clampedBits &other)
    {
        expand(std::max(mSize, other.mSize), 0);
        for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx)
        {
            auto first = 0ull, second = 0ull;
            if (block_idx < mBlocks)
                first = mData.get()[block_idx];
            if (block_idx < other.mBlocks)
                second = other.mData.get()[block_idx];
            mData.get()[block_idx] = first | second;
        }
    }
    clampedBits operator&(const clampedBits &other) const
    {
        clampedBits temp(std::max(mSize, other.mSize), 0);
        for (uint64_t block_idx = 0; block_idx < temp.mBlocks; ++block_idx)
        {
            auto first = 0ull, second = 0ull;
            if (block_idx < mBlocks)
                first = mData.get()[block_idx];
            if (block_idx < other.mBlocks)
                second = other.mData.get()[block_idx];
            temp.mData.get()[block_idx] = first & second;
        }
        return temp;
    }
    void operator&=(const clampedBits &other)
    {
        expand(std::max(mSize, other.mSize), 0);
        for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx)
        {
            auto first = 0ull, second = 0ull;
            if (block_idx < mSize)
                first = mData.get()[block_idx];
            if (block_idx < other.mSize)
                second = other.mData.get()[block_idx];
            mData.get()[block_idx] = first & second;
        }
    }
    clampedBits operator^(const clampedBits &other) const
    {
        clampedBits temp(std::max(mSize, other.mSize), 0);
        for (uint64_t block_idx = 0; block_idx < temp.mBlocks; ++block_idx)
        {
            auto first = 0ull, second = 0ull;
            if (block_idx < mBlocks)
                first = mData.get()[block_idx];
            if (block_idx < other.mBlocks)
                second = other.mData.get()[block_idx];
            temp.mData.get()[block_idx] = first ^ second;
        }
        return temp;
    }
    void operator^=(const clampedBits &other)
    {
        expand(std::max(this->mSize, other.mSize), 0);
        for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx)
        {
            auto first = 0ull, second = 0ull;
            if (block_idx < mBlocks)
                first = mData.get()[block_idx];
            if (block_idx < other.mBlocks)
                second = other.mData.get()[block_idx];
            mData.get()[block_idx] = first ^ second;
        }
    }
    clampedBits operator~() const
    {
        clampedBits temp(*this);
        std::for_each_n(temp.mData.get(), temp.mBlocks, [](uint64_t &item)
                        { item = ~item; });
        return temp;
    }
    friend std::ostream &operator<<(std::ostream &os, const clampedBits &bits)
    {
        for (uint64_t idx = 0; idx < bits.mSize; ++idx)
        {
            os << ((bits.mData[idx / 64] >> (idx % 64)) & 1);
        }
        return os;
    }
    void set_all(const unsigned int bit)
    {
        std::for_each_n(mData.get(), mBlocks, [=](uint64_t &item)
                        { item = (bit) ? ONES : 0; });
    }
    void set(const uint64_t position, const unsigned int bit)
    {
        if (position >= mSize)
            return;
        if (bit > 1)
            return;
        auto new_block = mData.get()[position / 64];
        new_block = (bit) ? (new_block | (bit << (position % 64))) : (new_block & (~(1ull << (position % 64))));
        mData.get()[position / 64] = new_block;
    }

    void expand(const uint64_t bit_count, const unsigned int filler = 0)
    {
        if (bit_count <= mSize)
            return;
        auto additional_size = bit_count - mSize;
        auto additional_blocks = additional_size / 64 + 1;
        auto newData = std::make_unique<uint64_t[]>(mBlocks + additional_blocks);
        std::fill_n(newData.get(), mBlocks + additional_blocks, filler ? ONES : 0);
        std::copy_n(mData.get(), mBlocks, newData.get());
        mData.swap(newData);
        mBlocks += additional_blocks;
        mSize += additional_size;
    }

    [[nodiscard]] unsigned int at(const uint64_t position) const
    {
        if (position >= mSize)
            return 0;
        return (mData.get()[position / 64] >> (position % 64)) & 1;
    }
    std::string str() const
    {
        std::string result;
        for (uint64_t idx = 0; idx < mSize; ++idx)
            result.push_back(((mData.get()[idx / 64]) >> (idx % 64)) & 1 ? '1' : '0');
        return result;
    }
    std::string base_str(unsigned int base = 10) const
    {
        if (base < 2 || base > 36)
            throw std::invalid_argument("Base must be between 2 and 36");

        if (mSize == 0)
            return "0";

        std::string result;
        clampedBits temp(*this);

        // Calculate the number of full blocks and bits in the last block
        uint64_t fullBlocks = mSize / 64;
        uint64_t lastBits = mSize % 64;

        while (temp.mSize > 0)
        {
            uint64_t remainder = 0;
            bool all_zero = true;

            // Process full blocks
            for (int64_t i = fullBlocks - 1; i >= 0; --i)
            {
                uint64_t current = remainder << 32 | (temp.mData[i] >> 32);
                uint64_t quotient = current / base;
                remainder = current % base;

                current = remainder << 32 | (temp.mData[i] & 0xFFFFFFFF);
                quotient = (quotient << 32) | (current / base);
                remainder = current % base;

                temp.mData[i] = quotient;

                if (quotient != 0)
                    all_zero = false;
            }

            // Process the last partial block
            if (lastBits > 0)
            {
                uint64_t mask = (1ULL << lastBits) - 1;
                uint64_t current = remainder << lastBits | (temp.mData[fullBlocks] & mask);
                uint64_t quotient = current / base;
                remainder = current % base;

                temp.mData[fullBlocks] = quotient & mask;

                if (quotient != 0)
                    all_zero = false;
            }

            char digit = remainder < 10 ? '0' + remainder : 'A' + remainder - 10;
            result = digit + result;

            if (all_zero)
                break;
        }

        return result;
    }
    const uint64_t &size() const { return mSize; }
};