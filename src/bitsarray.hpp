#pragma once
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <type_traits>
#include <memory>
#include <ostream>

class clampedBits{
    std::unique_ptr<uint64_t[]> mData;
    uint64_t mSize, mBlocks;
    template<class lambda_t> static void * for_each(const uint64_t* left, const uint64_t* right,
        const uint64_t l_size, const uint64_t r_size, uint64_t* storage, lambda_t lambda) {
        auto max_size = std::max(l_size, r_size);
        for(uint64_t idx = 0; idx < max_size; ++idx){
            auto item_l = (l_size > idx) ? left[idx] : 0;
            auto item_r = (r_size > idx) ? right[idx] : 0;
            storage[idx] = lambda(item_l, item_r);
        }
        return storage;
    }
public:
    static constexpr uint64_t ONES = std::numeric_limits<uint64_t>::max();
    explicit clampedBits(const uint64_t bit_count){
        mSize = bit_count; mBlocks = bit_count / 64 + 1;
        mData = std::make_unique<uint64_t[]>(mBlocks);
    }

    clampedBits(const clampedBits& other){
        mSize = other.mSize; mBlocks = other.mBlocks;
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::copy_n(other.mData.get(), mBlocks, mData.get());
    }
    clampedBits(clampedBits&&) noexcept = default;

    clampedBits& operator=(const clampedBits& other){
        mSize = other.mSize; mBlocks = other.mBlocks;
        mData = std::make_unique<uint64_t[]>(mBlocks);
        std::copy_n(other.mData.get(), mBlocks, mData.get());
        return *this;
    }
    clampedBits& operator=(clampedBits&&) noexcept = default;

    friend bool operator==(const clampedBits& lhs, const clampedBits& rhs){
        if(lhs.mSize != rhs.mSize) return false;
        for(uint64_t idx = 0; idx < lhs.mBlocks; ++idx){
            if(lhs.mData.get()[idx] ^ rhs.mData.get()[idx]) return false;
        }
        return true;
    }

    friend bool operator!=(const clampedBits& lhs, const clampedBits& rhs){
        return !(lhs == rhs);
    }

    unsigned int operator[](const uint64_t position) const{
        if(position >= mSize) throw std::out_of_range("");
        return (mData.get()[position / 64] >> (position % 64)) & 1;
    }

    clampedBits operator|(const clampedBits& other) const{
        clampedBits temp(std::max(mBlocks, other.mBlocks));
        for_each(mData.get(), other.mData.get(), mBlocks, other.mBlocks, temp.mData.get(), [](
                const uint64_t l, const uint64_t r){return l | r;}
        );
        return temp;
    }
    void operator|=(const clampedBits& other){
        expand(std::max(mSize, other.mSize), 0);
        for_each(mData.get(), other.mData.get(), mBlocks, other.mBlocks, mData.get(),
                 [](const uint64_t l, const uint64_t r){return l | r;});
    }
    clampedBits operator&(const clampedBits& other) const{
        clampedBits temp(std::max(mBlocks, other.mBlocks));
        for_each(mData.get(), other.mData.get(), mBlocks, other.mBlocks, temp.mData.get(), [](
                const uint64_t l, const uint64_t r){return l & r;}
                );
        return temp;
    }
    void operator&=(const clampedBits& other){
        expand(std::max(this->mSize, other.mSize));
        for_each(mData.get(), other.mData.get(), mBlocks, other.mBlocks, mData.get(), [](
                const uint64_t l, const uint64_t r){return l & r;}
        );
    }
    clampedBits operator^(const clampedBits& other) const{
        clampedBits temp(std::max(mSize, other.mSize));
        for_each(mData.get(), other.mData.get(), mBlocks, other.mBlocks, temp.mData.get(),
                 [](const uint64_t l, const uint64_t r){return l ^ r;});
        return temp;
    }
    void operator^=(const clampedBits& other){
        expand(std::max(mSize, other.mSize));
        for_each(mData.get(), other.mData.get(), mBlocks, other.mBlocks, mData.get(),
                 [](const uint64_t l, const uint64_t r){return l ^ r;});
    }
    clampedBits operator~() const {
        clampedBits temp(*this);
        std::for_each_n(temp.mData.get(), temp.mBlocks, [](uint64_t& item){ item = ~item; });
        return temp;
    }
    friend std::ostream& operator<<(std::ostream& os, const clampedBits& bits){
        for(uint64_t idx = 0; idx < bits.mSize; ++idx) {
            os << bits[idx];
        }
        return os;
    }
    void set_all(const unsigned int bit){
        std::for_each_n(mData.get(), mBlocks, [=](uint64_t& item){ item = (bit) ? ONES : 0;});
    }
    void set(const uint64_t position, const unsigned int bit) {
        if(position >= mSize) return;
        if(bit > 1) return;
        if(bit) mData.get()[position / 64] |= (bit) << (position % 64);
        else mData.get()[position / 64] &= ~(bit << (position % 64));
    }

    void expand(const uint64_t bit_count, const uint64_t filler = 0){
        if(bit_count <= mSize) return;
        auto additional_size = mSize - bit_count;
        auto additional_blocks = mBlocks - (additional_size + 63) % 64;
        auto newData = std::make_unique<uint64_t[]>(mBlocks + additional_blocks);
        std::copy_n(mData.get(), mBlocks, newData.get());
        std::fill_n(newData.get() + mBlocks, additional_blocks, filler);
        mData.swap(newData);
        mBlocks += additional_blocks;
        mSize += additional_size;
    }

    [[nodiscard]] unsigned int at(const uint64_t position) const {
        if(position >= mSize) return 0;
        return (mData.get()[position / 64] >> (position % 64)) & 1;
    }
};