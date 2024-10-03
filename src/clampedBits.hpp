#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <ostream>
#include <ranges>
#include <sstream>
#include <string>

class clampedBits {
  uint64_t mSize, mBlocks;
  std::unique_ptr<uint64_t[]> mData;
#pragma region private static funcs
  /**
   * @brief Checks is it a binary C string or not
   *
   * @param str
   * @return true
   * @return false
   */
  static bool is_binary_str(const char *str) {
    for (uint64_t idx = 0; str[idx] != '\0'; ++idx)
      if (str[idx] > '1' || str[idx] < '0')
        return false;
    return true;
  }
  static uint64_t get_front_bits(uint64_t number, uint64_t count) {
    return (ONES >> (64 - count)) & number;
  }
  static uint64_t get_last_bits(uint64_t number, uint64_t count) {
    return ~(1ull << (count)) & number;
  }
  static void trim(clampedBits &bits) {
    uint64_t count_top_zeros = 0, new_blocks = 1;
    for (int64_t bit_idx = bits.mSize - 1; !bits[bit_idx] && bit_idx >= 0;
         --bit_idx)
      count_top_zeros += 1;
    new_blocks = (bits.mSize - count_top_zeros) / 64 + 1;
    std::unique_ptr<uint64_t[]> new_data =
        std::make_unique<uint64_t[]>(new_blocks);
    std::copy_n(bits.mData.get(), new_blocks, new_data.get());
    bits.mBlocks = new_blocks;
    bits.mSize -= count_top_zeros;
    bits.mData.swap(new_data);
  }
#pragma endregion

public:
#pragma region usefull staff
  /**
   * @brief Block of data filled with `1`
   *
   */
  static constexpr uint64_t ONES = std::numeric_limits<uint64_t>::max();
  /**
   * @brief Count how many bits needs to represent an integer
   *
   * @param number Just an integer
   * @return uint64_t
   */
  static uint64_t bitlen(int64_t number) {
    if (number < 3)
      return number == 2 ? 2 : 1;
    else
      return static_cast<uint64_t>(std::ceil(std::log2(std::abs(number))));
  }
#pragma endregion

#pragma region constructors
  clampedBits(const uint64_t bit_count, unsigned int filler)
      : mSize(bit_count), mBlocks(bit_count / 64 + 1) {
    mData = std::make_unique<uint64_t[]>(mBlocks);
    std::fill_n(mData.get(), mBlocks, (filler) ? ONES : 0);
  }

  clampedBits(const char *binary_string)
      : mSize(is_binary_str(binary_string) ? std::strlen(binary_string) : 0),
        mBlocks(mSize / 64 + 1) {
    mData = std::make_unique<uint64_t[]>(mBlocks);
    for (uint64_t idx = 0; idx < mSize; ++idx)
      mData[idx / 64] |=
          (static_cast<uint64_t>(binary_string[idx] - '0') << (idx % 64));
  }

  clampedBits(const uint64_t bits_data)
      : mSize(clampedBits::bitlen(bits_data)), mBlocks(1),
        mData(new uint64_t[1]{bits_data}) {}

  clampedBits(const clampedBits &other)
      : mSize(other.mSize), mBlocks(other.mBlocks) {
    mData = std::make_unique<uint64_t[]>(mBlocks);
    std::copy_n(other.mData.get(), mBlocks, mData.get());
  }

  clampedBits(clampedBits &&) noexcept = default;

  clampedBits &operator=(const clampedBits &other) {
    mSize = other.mSize;
    mBlocks = other.mBlocks;
    mData = std::make_unique<uint64_t[]>(mBlocks);
    std::copy_n(other.mData.get(), mBlocks, mData.get());
    return *this;
  }
  clampedBits &operator=(clampedBits &&) noexcept = default;
#pragma endregion

#pragma region operators

  [[nodiscard]] unsigned int operator[](const uint64_t bit_index) const {
    return bit_index >= mSize ? 0
                              : (mData[bit_index / 64] >> (bit_index % 64)) & 1;
  }

  friend std::ostream &operator<<(std::ostream &os, const clampedBits &bits) {
    for (uint64_t bit_pos = 0; bit_pos < bits.mSize; ++bit_pos)
      os << bits[bit_pos];
    return os;
  }

#pragma endregion

#pragma region unary operators

  clampedBits operator+(const clampedBits &other) const {
    const uint64_t max_size = std::max(mSize, other.mSize);
    clampedBits result(max_size + 1, 0);
    uint64_t carry = 0;

    for (uint64_t block_idx = 0; block_idx < result.mBlocks; ++block_idx) {
      uint64_t a = (block_idx < mBlocks) ? mData[block_idx] : 0;
      uint64_t b = (block_idx < other.mBlocks) ? other.mData[block_idx] : 0;

      uint64_t sum = a + b + carry;
      result.mData[block_idx] = sum;
      carry = (sum < a) || (sum < b);
    }
    clampedBits::trim(result);
    return result;
  }

  clampedBits operator-(const clampedBits &other) const {
    if (*this < other)
      return clampedBits(1, 0);

    clampedBits result(mSize, 0);
    int64_t borrow = 0;

    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      uint64_t a = mData[block_idx];
      uint64_t b = (block_idx < other.mBlocks) ? other.mData[block_idx] : 0;

      uint64_t diff = a - b - borrow;
      result.mData[block_idx] = diff;
      borrow = (a < b) || (static_cast<int64_t>(a) < borrow);
    }
    clampedBits::trim(result);
    return result;
  }

#pragma endregion

#pragma region eq operators
  friend std::strong_ordering operator<=>(const clampedBits &lhs,
                                          const clampedBits &rhs) {
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
  friend bool operator==(const clampedBits &lhs, const clampedBits &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }
  friend bool operator!=(const clampedBits &lhs, const clampedBits &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator==(const clampedBits &lhs, uint64_t rhs) {
    return lhs == clampedBits(rhs);
  }
  friend bool operator!=(const clampedBits &lhs, uint64_t rhs) {
    return !(lhs == rhs);
  }

  friend bool operator==(const clampedBits &lhs, double rhs) {
    return lhs == clampedBits(static_cast<uint64_t>(rhs));
  }
  friend bool operator!=(const clampedBits &lhs, double rhs) {
    return !(lhs == rhs);
  }

  friend bool operator==(const clampedBits &lhs, const char *binary_str) {
    return lhs == clampedBits(binary_str);
  }
  friend bool operator!=(const clampedBits &lhs, const char *binary_str) {
    return !(lhs == binary_str);
  }
#pragma endregion

#pragma region comp operators
  friend bool operator>(const clampedBits &lhs, const clampedBits &rhs) {
    return lhs <=> rhs == std::strong_ordering::greater;
  }
  friend bool operator>=(const clampedBits &lhs, const clampedBits &rhs) {
    auto val = lhs <=> rhs;
    return val == std::strong_ordering::greater ||
           val == std::strong_ordering::equal;
  }

  friend bool operator<(const clampedBits &lhs, const clampedBits &rhs) {
    return lhs <=> rhs == std::strong_ordering::less;
  }
  friend bool operator<=(const clampedBits &lhs, const clampedBits &rhs) {
    auto val = lhs <=> rhs;
    return val == std::strong_ordering::less ||
           val == std::strong_ordering::equal;
  }
#pragma endregion

#pragma region usefull methods
  /**
   * @brief Позволяет установить конкретный бит в конкретное значение.
   *
   * @param position Индекс интересующего бита. Если больше размера контейнера,
   * будет проигнорирован.
   * @param bit Значение интересующего нас бита.
   */
  void set(uint64_t position, unsigned int bit) {
    if (position >= mSize)
      return;
    uint64_t new_state = 1ull << (position % 64);
    if (bit)
      mData[position / 64] |= new_state;
    else
      mData[position / 64] &= ~new_state;
  }

  /**
   * @brief Sets all bits to given value
   *
   * @param bit Value that we want to set in all position
   */
  void set_all(const unsigned int bit) {
    uint64_t block = (bit) ? ONES : 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx)
      mData[block_idx] = block;
  }

  /**
   * @brief Allows you to get the value of a specific bit.
   *
   * @param position The position of the bit of interest. A value larger than
   * the container size will be ignored.
   * @return unsigned int
   */
  [[nodiscard]] unsigned int at(const uint64_t position) const {
    if (position >= mSize)
      return 0;
    return (mData[position / 64] >> (position % 64)) & 1;
  }

  /**
   * @brief Transforms the bit container into a string with bit values.
   *
   * @return std::string
   */
  std::string str() const {
    std::string result(mSize, '0');
    for (uint64_t idx = 0; idx < mSize; ++idx)
      result.at(idx) = at(idx) + '0';
    return result;
  }

  /**
   * \brief Allows you to increase the size of the bit container.
   * \param new_size The number of bits to have in the container (larger than
   * the current container size). \param filler The value that will be used to
   * fill the new space in the container (default is 0).
   */
  void expand(const uint64_t new_size, const unsigned int filler = 0) {
    if (new_size <= mSize)
      return;
    uint64_t new_blocks = new_size / 64 + 1;
    std::unique_ptr<uint64_t[]> new_data =
        std::make_unique<uint64_t[]>(new_blocks);
    std::copy_n(mData.get(), mBlocks, new_data.get());
    if (filler)
      new_data[mBlocks - 1] |= ONES << (mSize % 64);
    std::fill_n(new_data.get() + mBlocks, new_blocks - mBlocks,
                filler ? ONES : 0);
    mData.swap(new_data);
    mBlocks = new_blocks;
    mSize = new_size;
  }

  /**
   * @brief Returns count of bits in container.
   *
   * @return const uint64_t&
   */
  const uint64_t &size() const { return mSize; }
#pragma endregion

#pragma region bits operators

  clampedBits operator<<(const uint64_t count) const {
    clampedBits result(mSize + count, 0);
    uint64_t full_block_move = count / 64, bits_move = count % 64;
    uint64_t carry = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; block_idx++) {
      result.mData[block_idx + full_block_move] =
          (mData[block_idx] << bits_move) | carry;
      carry = get_front_bits(mData[block_idx], bits_move);
    }
    return result;
  }
  void operator<<=(const uint64_t count) {
    uint64_t new_blocks = (mSize + count) / 64 + 1;
    std::unique_ptr<uint64_t[]> new_data =
        std::make_unique<uint64_t[]>(new_blocks);
    uint64_t full_block_move = count / 64, bits_move = count % 64;
    uint64_t carry = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      new_data[block_idx + full_block_move] =
          (mData[block_idx] << bits_move) | carry;
      carry = get_front_bits(mData[block_idx], bits_move);
    }
    mData.swap(new_data);
    mBlocks = new_blocks;
    mSize += count;
  }

  clampedBits operator>>(const uint64_t count) const {
    if (count >= mSize)
      return clampedBits(1, 0);
    clampedBits result(mSize - count, 0);
    uint64_t full_block_move = count / 64, bits_move = count % 64;
    uint64_t carry = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; block_idx++) {
      result.mData[block_idx] =
          (mData[block_idx + full_block_move] >> bits_move) | carry;
      carry = get_last_bits(mData[block_idx + full_block_move], bits_move);
    }
    return result;
  }
  void operator>>=(const uint64_t count) {
    if (count >= mSize) {
      *this = clampedBits(1, 0);
      return;
    }
    uint64_t new_blocks = (mSize - count) / 64 + 1;
    std::unique_ptr<uint64_t[]> new_data =
        std::make_unique<uint64_t[]>(new_blocks);
    uint64_t full_block_move = count / 64, bits_move = count % 64;
    uint64_t carry = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      new_data[block_idx] =
          (mData[block_idx + full_block_move] >> bits_move) | carry;
      carry = get_last_bits(mData[block_idx + full_block_move], bits_move);
    }
    mData.swap(new_data);
    mBlocks = new_blocks;
    mSize -= count;
  }

  clampedBits operator|(const clampedBits &other) const {
    clampedBits result(std::max(mSize, other.mSize), 0);
    uint64_t left = 0, right = 0;
    for (uint64_t block_idx = 0; block_idx < result.mBlocks; ++block_idx) {
      left = block_idx < mBlocks ? mData[block_idx] : 0;
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      result.mData[block_idx] = left | right;
    }
    return result;
  }
  void operator|=(const clampedBits &other) {
    expand(std::max(mSize, other.mSize), 0);
    uint64_t left = 0, right = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      left = block_idx < mBlocks ? mData[block_idx] : 0;
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      mData[block_idx] = left | right;
    }
  }

  clampedBits operator&(const clampedBits &other) const {
    clampedBits result(std::max(mSize, other.mSize), 0);
    uint64_t left = 0, right = 0;
    for (uint64_t block_idx = 0; block_idx < result.mBlocks; ++block_idx) {
      left = block_idx < mBlocks ? mData[block_idx] : 0;
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      result.mData[block_idx] = left & right;
    }
    return result;
  }
  void operator&=(const clampedBits &other) {
    expand(std::max(mSize, other.mSize), 0);
    uint64_t left = 0, right = 0;
    for (uint64_t block_idx = 0; block_idx < mBlocks; ++block_idx) {
      left = block_idx < mBlocks ? mData[block_idx] : 0;
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      mData[block_idx] = left & right;
    }
  }

  clampedBits operator^(const clampedBits &other) const {
    clampedBits result(std::max(mSize, other.mSize), 0);
    uint64_t left = 0, right = 0;

    for (uint64_t block_idx = 0; block_idx < result.mBlocks; ++block_idx) {
      left = block_idx < mBlocks ? mData[block_idx] : 0;
      right = block_idx < other.mBlocks ? other.mData[block_idx] : 0;
      result.mData[block_idx] = left ^ right;
    }

    return result;
  }
#pragma region convert_to_base_method
public:
  /**
   * @brief Converts the bit array to a string representation in the specified
   * base.
   *
   * @param base The base for conversion (default is 10).
   * @return std::string The string representation of the bit array in the
   * specified base.
   */
  std::string convert_to_base(const unsigned int base = 10) const {
    if (base < 2 || base > 36) {
      throw std::invalid_argument("Base must be between 2 and 36");
    }

    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    // Проверка на ноль
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

    // Копия текущих битов для манипуляций
    clampedBits num(*this);
    std::stringstream ss;

    while (num != clampedBits(1, 0)) { // Пока num не равен нулю
      uint64_t remainder = 0;
      // Результат деления будет храниться обратно в num
      // Начинаем с самого старшего блока
      for (int64_t block_idx = num.mBlocks - 1; block_idx >= 0; --block_idx) {
        // Смещаем остаток на 64 бита влево и добавляем текущий блок
        __uint128_t temp =
            (__uint128_t(remainder) << 64) | num.mData[block_idx];
        // Делим на base
        num.mData[block_idx] = temp / base;
        // Остаток
        remainder = temp % base;
      }

      // Убираем ведущие нули блоков
      clampedBits::trim(num);

      // Добавляем символ остатка в строку
      ss << digits[remainder];
    }

    // Переворачиваем строку, так как остатки записываются в обратном порядке
    std::string result = ss.str();
    std::reverse(result.begin(), result.end());
    return result;
  }
#pragma endregion
};
