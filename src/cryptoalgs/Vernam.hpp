#pragma once
#ifndef _VERNAM_ALG_HPP
#define _VERNAM_ALG_HPP

#include "../src/RadInt.hpp"

cpp_int Vernam_key(const cpp_int& Number) {
	size_t len = functions::bitCount(Number) + 1;
	return integers::getRandomBits(len);
}
cpp_int Vernam_alg(const cpp_int& message, const cpp_int& key) {
	return message ^ key;
}

#endif