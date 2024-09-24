#pragma once
#ifndef _ANONIM_SIGN_HPP
#define _ANONIM_SIGN_HPP

#include "../src/RadInt.hpp"
#include "../src/cryptoalgs/RSA.hpp"
#include "boost/compute/detail/sha1.hpp"
#include <deque>
#include <map>

template<typename T>
class Blank {
	T data;
	cpp_int hash = 0;
    Blank(const T& data) : data(data), hash(
        functions::fromHex(boost::compute::detail::sha1(std::string(data)))
    ) {}
    Blank(const Blank& other) : data(other.data), hash(other.hash) {}
    const cpp_int& getHash() const { return hash; }
    const T& getData() const { return data; }
};

class User {
    cpp_int randomValue, voteInfo, hash, sign;
public:
    User() : randomValue(integers::getRandomInteger(512)) {}

    template <typename T>
    void vote(const Blank<T>& blank) {
        voteInfo = (randomValue << 512) + blank.getHash();
        hash = functions::fromHex(boost::compute::detail::sha1(voteInfo.str()));
    }
    const cpp_int& getVote() const { return voteInfo; }
    const cpp_int& getHash() const { return hash; }
    const cpp_int& getSign() const { return sign; }
    
    void setSign(const cpp_int& sign) {
        if (this->sign == 0) this->sign = sign;
    }
};

using votePair = std::array<cpp_int, 2>;

class Server {
private:
    RSA rsa;
    std::deque<votePair> votes;
public:
    Server() {
        auto initPair = RSA::createPair(1024);
        rsa = RSA(initPair[0], initPair[1]);
    }
    void signVote(User& user) {
        if (user.getVote() == 0 || user.getSign() != 0) {
            throw std::invalid_argument("This vote can't be signed");
        }
        cpp_int r, ir, h, h_, s, s_;
        auto invPair = rsa.createInversePair(rsa.getN());
        r = invPair[0], ir = invPair[1];
        h = user.getHash();
        h_ = functions::mulMod(h,
            functions::powMod(r, rsa.getKey(0), rsa.getN()),
            rsa.getN());
        s_ = functions::powMod(h_, rsa.getKey(1), rsa.getN());
        s = functions::mulMod(s_, ir, rsa.getN());
        if (h == functions::powMod(s, rsa.getKey(0), rsa.getN())) {
            votes.push_back({ user.getVote(), s });
            user.setSign(s);
        }
        else {
            throw std::runtime_error("Error while crypt");
        }
    }

    template<typename T>
    const std::map<Blank<T>, uint16_t>
        calculateVotes(const std::deque<Blank<T>>& blanks) const {
        cpp_int blankHash, voteHash;
        std::map<Blank<T>, uint16_t> resMap;
        for (const Blank<T>& blank : blanks)
            blankHash = blank.getHash();
        for (const votePair& voteP : votes) {
            voteHash = voteP[0] % (cpp_int(1) << 512 - 1);
            if (voteHash == blankHash) ++resMap[blank];
        }
    }

};


/* From here starts class Server impl*/


#endif