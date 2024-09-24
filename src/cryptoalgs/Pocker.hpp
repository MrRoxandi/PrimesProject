#pragma once
#ifndef POCKER_HPP
#define POCKER_HPP
#include "../src/RadInt.hpp"
#include "RSA.hpp"
#include <string>
#include <sstream>
#include <deque>
enum pockerNum { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, J, Q, K, T };
enum pockerLit { D = 100, H = 101, S = 110, C = 111 };

class Card {
private:
	cpp_int literal = 0, number = 0;
public:
	Card(pockerLit lit, pockerNum num): literal(lit), number(num) {}
	Card(const Card& other): literal(other.literal), number(other.number)  {}
	cpp_int getLiteral() const { return literal; }
	cpp_int getNumber() const { return number;  }
	std::string str() const {
		std::stringstream stream;
		if (literal > 111 || literal < 100) stream << literal;
		else {
			pockerLit lit = literal.convert_to<pockerLit>();
			switch (lit){
			case D:
				stream << 'D'; break;
			case H:
				stream << 'H'; break;
			case S:
				stream << 'S'; break;
			case C:
				stream << 'C'; break;
			}
		}
		stream << '|';
		if (number <= 2 || number > 14) stream << number;
		else {
			pockerNum num = number.convert_to<pockerNum>();
			switch (num){
			case J:
				stream << 'J'; break;
			case Q:
				stream << 'Q'; break;
			case K:
				stream << 'K'; break;
			case T:
				stream << 'T'; break;
			default:
				stream << number; break;
			}
		}
		return stream.str();
	}
	
	void useKey(const cpp_int& N, const cpp_int& p) {
		literal = functions::powMod(literal, N, p);
		number = functions::powMod(number, N, p);
	}
	friend bool operator==(const Card& lhs, const Card& rhs) {
		return lhs.literal == rhs.literal && lhs.number == rhs.number;
	}
};

class Deck {
private:
	std::deque<Card> loadout;
public:
	Deck() {
		for (auto num = 2; num <= 13; ++num)
			for (auto lit : { 100, 101, 110, 111 })
				loadout.emplace_back(static_cast<pockerLit>(lit), static_cast<pockerNum>(num));
	}

	void insertCard(const Card& card) {
		loadout.emplace_back(card);
	}
	Card getCard() {
		Card card = loadout.at(loadout.size() - 1);
		std::erase(loadout, card);
		return card;
	}
	void shuffle() {
		boost::random::mt19937_64 enginge{ std::random_device{}() };
		std::shuffle(loadout.begin(), loadout.end(), enginge);
	}

	void useKey(const cpp_int& number, const cpp_int& p) {
		for (auto& card : loadout) card.useKey(number, p);
	}


};

class Player {
private:
	std::deque<Card> loadout;
	std::array<cpp_int, 2> keys;
public:
	Player(const cpp_int& p): loadout({}), keys(RSA::createInversePair(p - 1)){}
	const std::deque<Card>& getLoadout() const { return loadout; }
	void insertCard(const Card& card) {
		loadout.emplace_back(card);
	}
	void useKey(const cpp_int& key, const cpp_int& p) {
		for (auto& card : loadout) {
			card.useKey(key, p);
		}
	}
	cpp_int getKey(size_t pos) const {
		return keys.at(pos);
	}
};
#endif // !POCKER_HPP
