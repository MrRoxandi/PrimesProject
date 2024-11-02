#include "src/bits.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
bits::container multiply(const bits::container &left,
                         const bits::container &right) {
  bits::container result(left.size() + right.size(), 0);
  for (int64_t i = right.size(); i >= 0; --i)
    if (right[i])
      result += (left << i);
  return result;
}

int main() {
  bits::container left = 6263627420176, right = 6263627420176;
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left += 123;
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  left = multiply(left, 5);
  std::cout << left.convert_to_base(10) << '\n';
}
