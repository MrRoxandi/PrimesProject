#pragma once
#ifndef CLAMPED_BITS_HPP
#define CLAMPED_BITS_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <ostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>

namespace bits {

// Utility functions
namespace detail {
constexpr uint64_t ONES = std::numeric_limits<uint64_t>::max();

bool is_binary_str(const char *str) {
  if (!str)
    return false;
  return std::all_of(str, str + std::strlen(str),
                     [](char c) { return c == '0' || c == '1'; });
}

uint64_t get_front_bits(uint64_t number, uint64_t count) {
  if (count >= 64)
    return number;
  return number >> (64 - count);
}

uint64_t get_last_bits(uint64_t number, uint64_t count) {
  if (count >= 64)
    return number;
  return number & ((1ULL << count) - 1);
}

static uint64_t calculate_blocks(uint64_t bits) {
  return (bits + 63) / 64; // Ceiling division
}

uint64_t bitlen(std::signed_integral auto number) {
  if (number < 1)
    return 1;
  return static_cast<uint64_t>(std::ceil(std::log2(std::abs(number)))) + 1;
}

uint64_t bitlen(std::unsigned_integral auto number) {
  if (number < 1)
    return 1;
  return static_cast<uint64_t>(std::ceil(std::log2(number))) + 1;
}

} // namespace detail

class container {
private:
  uint64_t mSize;
  uint64_t mBlocks;
  std::unique_ptr<uint64_t[]> mData;

  static void trim(container &bits) {
    if (bits.mSize <= 1)
      return;

    uint64_t count_top_zeros = 0;
    for (int64_t bit_idx = bits.mSize - 1; bit_idx >= 0 && !bits[bit_idx];
         --bit_idx) {
      count_top_zeros++;
    }

    if (count_top_zeros == 0)
      return;

    uint64_t new_size = bits.mSize - count_top_zeros;
    uint64_t new_blocks = detail::calculate_blocks(new_size);

    if (new_blocks == bits.mBlocks) {
      bits.mSize = new_size;
      return;
    }

    auto new_data = std::make_unique<uint64_t[]>(new_blocks);
    std::copy_n(bits.mData.get(), new_blocks, new_data.get());
    bits.mBlocks = new_blocks;
    bits.mSize = new_size;
    bits.mData = std::move(new_data);
  }

  static void shift_left_blocks(uint64_t *data, uint64_t blocks,
                                uint64_t shift_blocks) {
    if (shift_blocks >= blocks) {
      std::fill_n(data, blocks, 0);
      return;
    }
    std::copy_backward(data, data + (blocks - shift_blocks), data + blocks);
    std::fill_n(data, shift_blocks, 0);
  }

  static void shift_right_blocks(uint64_t *data, uint64_t blocks,
                                 uint64_t shift_blocks) {
    if (shift_blocks >= blocks) {
      std::fill_n(data, blocks, 0);
      return;
    }
    std::copy(data + shift_blocks, data + blocks, data);
    std::fill_n(data + (blocks - shift_blocks), shift_blocks, 0);
  }

public:
  // Constructors
  explicit container(uint64_t bit_count, unsigned int filler)
      : mSize(bit_count), mBlocks(detail::calculate_blocks(bit_count)),
        mData(std::make_unique<uint64_t[]>(mBlocks)) {
    std::fill_n(mData.get(), mBlocks, filler ? detail::ONES : 0);
  }

  container(const char *binary_string) : mSize(0), mBlocks(0), mData(nullptr) {
    if (!binary_string || !detail::is_binary_str(binary_string)) {
      throw std::invalid_argument("Invalid binary string");
    }

    mSize = std::strlen(binary_string);
    mBlocks = detail::calculate_blocks(mSize);
    mData = std::make_unique<uint64_t[]>(mBlocks);
    std::fill_n(mData.get(), mBlocks, 0);

    for (uint64_t idx = 0; idx < mSize; ++idx) {
      if (binary_string[mSize - idx - 1] == '1') {
        mData[idx / 64] |= (1ULL << (idx % 64));
      }
    }
  }

  container(std::integral auto bits_data)
      : container(detail::bitlen(bits_data), 0) {
    mData[0] = bits_data;
  }

  // Copy and move
  container(const container &other)
      : mSize(other.mSize), mBlocks(other.mBlocks),
        mData(std::make_unique<uint64_t[]>(mBlocks)) {
    std::copy_n(other.mData.get(), mBlocks, mData.get());
  }

  container(container &&) noexcept = default;

  container &operator=(const container &other) {
    if (this != &other) {
      container temp(other);
      swap(temp);
    }
    return *this;
  }
  container &operator=(container &&) noexcept = default;

  void swap(container &other) noexcept {
    std::swap(mSize, other.mSize);
    std::swap(mBlocks, other.mBlocks);
    mData.swap(other.mData);
  }

  // Utility methods

  // Compare operators
  friend std::strong_ordering operator<=>(const container &lhs,
                                          const container &rhs) {
    uint64_t max_blocks = std::max(lhs.mBlocks, rhs.mBlocks);
    uint64_t left = 0, right = 0;
    for (int64_t block_idx = max_blocks - 1; block_idx >= 0; block_idx -= 1) {
      left = static_cast<uint64_t>(block_idx) < lhs.mBlocks
                 ? lhs.mData[block_idx]
                 : 0;
      right = static_cast<uint64_t>(block_idx) < rhs.mBlocks
                  ? rhs.mData[block_idx]
                  : 0;
      if (left < right)
        return std::strong_ordering::less;
      if (left > right)
        return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
  }

  friend bool operator==(const container &lhs, const container &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }
  friend bool operator!=(const container &lhs, const container &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator==(const container &lhs, std::integral auto rhs) {
    return lhs == container(rhs);
  }
  friend bool operator!=(const container &lhs, std::integral auto rhs) {
    return !(lhs == rhs);
  }

  friend bool operator==(const container &lhs, std::floating_point auto rhs) {
    return lhs == container(static_cast<uint64_t>(rhs));
  }
  friend bool operator!=(const container &lhs, std::floating_point auto rhs) {
    return !(lhs == rhs);
  }

  friend bool operator==(const container &lhs, const char *binary_str) {
    return lhs == container(binary_str);
  }
  friend bool operator!=(const container &lhs, const char *binary_str) {
    return !(lhs == binary_str);
  }

  // Operators
  unsigned int operator[](uint64_t bit_index) const {
    return bit_index >= mSize ? 0
                              : (mData[bit_index / 64] >> (bit_index % 64)) & 1;
  }

  // Assignment operators
  container &operator+=(const container &other) {
    uint64_t max_size = std::max(mSize, other.mSize) + 1;
    expand(max_size);

    uint64_t carry = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      __uint128_t sum =
          static_cast<__uint128_t>(mData[block_idx]) +
          (block_idx < other.mBlocks ? other.mData[block_idx] : 0) + carry;
      mData[block_idx] = static_cast<uint64_t>(sum);
      carry = static_cast<uint64_t>(sum >> 64);
    }

    trim(*this);
    return *this;
  }

  container &operator-=(const container &other) {
    if (*this < other)
      *this = container(1, 0); // Устанавливаем ноль
    uint64_t borrow = 0, left = 0, right = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      left = mData[block_idx];
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      __int128 temp = static_cast<__uint128_t>(left) - right - borrow;
      if (temp < 0) {
        mData[block_idx] = static_cast<uint64_t>(temp + (__int128_t(1) << 64));
        borrow = 1;
      } else {
        mData[block_idx] = static_cast<uint64_t>(temp);
        borrow = 0;
      }
    }
    container::trim(*this);
    return *this;
  }

  container &operator*=(const container &other) {
    if (*this == 0ull || other == 0ull) {
      *this = container(1, 0);
      return *this;
    }
    container result(mSize + other.mSize, 0);
    {
      container temp(*this);
      for (uint64_t i = 0; i < other.mSize; ++i) {
        if (other[i])
          result += temp;
        temp <<= 1;
      }
    }
    *this = std::move(result);
    trim(*this);
    return *this;
  }

  container &operator/=(const container &divisor) {
    if (divisor == 0ull) {
      throw std::invalid_argument("Division by zero");
    }

    if (*this < divisor) {
      *this = container(1, 0);
      return *this;
    }

    container quotient(mSize, 0);
    container remainder(*this);
    container temp_divisor(divisor);

    // Нормализация делителя
    int shift = mSize - temp_divisor.mSize;
    temp_divisor <<= shift;

    // Пока остаток больше или равен исходному делителю
    while (shift >= 0) {
      if (remainder >= temp_divisor) {
        remainder -= temp_divisor;
        quotient.set(shift, 1);
      }
      temp_divisor >>= 1;
      --shift;
    }

    *this = std::move(quotient);
    trim(*this);
    return *this;
  }

  container &operator%=(const container &divisor) {
    if (divisor == 0ull) {
      throw std::invalid_argument("Division by zero");
    }

    if (*this < divisor) {
      return *this;
    }

    container temp_divisor(divisor);

    // Нормализация делителя
    int shift = mSize - temp_divisor.mSize;
    temp_divisor <<= shift;

    // Пока остаток больше или равен исходному делителю
    while (shift >= 0) {
      if (*this >= temp_divisor) {
        *this -= temp_divisor;
      }
      temp_divisor >>= 1;
      --shift;
    }

    trim(*this);
    return *this;
  }

  // Utility methods
  void expand(uint64_t new_size, unsigned int filler = 0) {
    if (new_size <= mSize)
      return;

    uint64_t new_blocks = detail::calculate_blocks(new_size);
    if (new_blocks == mBlocks) {
      mSize = new_size;
      return;
    }

    auto new_data = std::make_unique<uint64_t[]>(new_blocks);
    std::copy_n(mData.get(), mBlocks, new_data.get());

    if (filler) {
      std::fill_n(new_data.get() + mBlocks, new_blocks - mBlocks, detail::ONES);
      // Fix the last partial block in the original data if necessary
      if (mSize % 64 != 0) {
        new_data[mBlocks - 1] |= detail::ONES << (mSize % 64);
      }
    } else {
      std::fill_n(new_data.get() + mBlocks, new_blocks - mBlocks, 0);
    }

    mData = std::move(new_data);
    mBlocks = new_blocks;
    mSize = new_size;
  }

  [[nodiscard]] uint64_t size() const { return mSize; }

  void set(uint64_t position, unsigned int bit) {
    if (position >= mSize)
      return;
    auto new_bit = 1ull << (position % 64);
    switch (bit) {
    case 0:
      mData[position / 64] &= ~(new_bit);
      break;
    default:
      mData[position / 64] |= new_bit;
      break;
    }
  }

  void set_all(unsigned int bit) {
    auto new_block = (bit) ? detail::ONES : 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx)
      mData[block_idx] = new_block;
  }
  // Bits manip
  container operator<<(uint64_t count) const {
    if (count == 0)
      return *this;
    if (count >= std::numeric_limits<uint64_t>::max() - mSize) {
      throw std::overflow_error("Shift would cause overflow");
    }
    uint64_t new_size = mSize + count;
    uint64_t new_blocks = detail::calculate_blocks(new_size);
    container result(new_size, 0);

    if (count >= new_size)
      return result; // All bits are zero

    uint64_t block_shift = count / 64;
    uint64_t bit_shift = count % 64;

    // Only full blocks shift
    if (bit_shift == 0) {
      std::copy_n(mData.get(), mBlocks, result.mData.get() + block_shift);
      return result;
    }

    // basic shift
    uint64_t carry = 0;
    for (uint64_t i = 0; i < mBlocks; ++i) {
      uint64_t current = mData[i];
      result.mData[i + block_shift] = carry | (current << bit_shift);
      carry = (bit_shift != 0) ? (current >> (64 - bit_shift)) : 0;
    }
    if (carry && (block_shift + mBlocks < new_blocks)) {
      result.mData[block_shift + mBlocks] = carry;
    }

    return result;
  }
  container operator>>(uint64_t count) const {
    if (count == 0)
      return *this;
    if (count >= mSize)
      return container(1, 0);

    uint64_t new_size = mSize - count;
    uint64_t new_blocks = detail::calculate_blocks(new_size);
    container result(new_size, 0);

    uint64_t block_shift = count / 64;
    uint64_t bit_shift = count % 64;

    if (bit_shift == 0) {
      std::copy_n(mData.get() + block_shift, new_blocks, result.mData.get());
      return result;
    }

    for (uint64_t i = 0; i < new_blocks; ++i) {
      uint64_t current = mData[i + block_shift];
      uint64_t next =
          (i + block_shift + 1 < mBlocks) ? mData[i + block_shift + 1] : 0;

      result.mData[i] =
          (current >> bit_shift) |
          ((next & ((1ULL << bit_shift) - 1)) << (64 - bit_shift));
    }

    return result;
  }

  container &operator<<=(uint64_t count) {
    *this = *this << count;
    return *this;
  }

  container &operator>>=(uint64_t count) {
    *this = *this >> count;
    return *this;
  }
  // Binary operators
private:
  container &any_binary_operator(const container &other, auto binary_operator) {
    expand(std::max(mSize, other.mSize), 0);
    uint64_t left = 0, right = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      left = block_idx < mBlocks ? mData[block_idx] : 0;
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      mData[block_idx] = binary_operator(left, right);
    }
    trim(*this);
    return *this;
  }

public:
  container &operator|=(const container &other) {
    return any_binary_operator(other, [](auto l, auto r) { return l | r; });
  }
  container &operator&=(const container &other) {
    return any_binary_operator(other, [](auto l, auto r) { return l & r; });
  }

  container operator^(const container &other) const {
    container result(*this);
    result.any_binary_operator(other, [](auto l, auto r) { return l ^ r; });
    return result;
  }

  container operator~() const {
    container result(*this);
    for (uint64_t idx = 0; idx < result.mBlocks; ++idx)
      result.mData[idx] = ~result.mData[idx];
    return result;
  }

  std::string convert_to_base(const unsigned int base = 10) const {
    if (base < 2 || base > 36) {
      throw std::invalid_argument("Base must be between 2 and 36");
    }

    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    bool is_zero = true;
    for (uint64_t i = 0; i < mBlocks; ++i) {
      if (mData[i] != 0) {
        is_zero = false;
        break;
      }
    }
    if (is_zero) {
      return "0";
    }

    container num(*this);
    std::stringstream ss;

    while (num != container(1, 0)) {
      uint64_t remainder = 0;
      for (int64_t block_idx = num.mBlocks - 1; block_idx >= 0; --block_idx) {
        __uint128_t temp =
            (__uint128_t(remainder) << 64) | num.mData[block_idx];
        num.mData[block_idx] = temp / base;
        remainder = temp % base;
      }

      container::trim(num);

      ss << digits[remainder];
    }

    std::string result = ss.str();
    std::reverse(result.begin(), result.end());
    return result;
  }

  friend inline std::ostream &operator<<(std::ostream &os,
                                         const container &item) {

    for (int64_t bit_pos = item.mSize - 1; bit_pos >= 0; --bit_pos)
      os << item[bit_pos];
    return os;
  }
};

// Free functions
inline container operator+(container lhs, const container &rhs) {
  lhs += rhs;
  return lhs;
}

inline container operator-(container lhs, const container &rhs) {
  lhs -= rhs;
  return lhs;
}

inline container operator|(container lhs, const container &rhs) {
  lhs |= rhs;
  return lhs;
}

inline container operator&(container lhs, const container &rhs) {
  lhs &= rhs;
  return lhs;
}

inline container operator*(container lhs, const container &rhs) {
  lhs *= rhs;
  return lhs;
}

inline container operator/(container lhs, const container &rhs) {
  lhs /= rhs;
  return lhs;
}

inline container operator%(container lhs, const container &rhs) {
  lhs %= rhs;
  return lhs;
}

} // namespace bits

#endif
