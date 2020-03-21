//
// Created by Leonid  on 21.03.2020.
//

#ifndef Nickname_hpp
#define Nickname_hpp

#include "oatpp/core/Types.hpp"

#include <vector>
#include <random>

class Nickname {
public:

  static constexpr int ADJECTIVES_SIZE = 103;
  static const char* const ADJECTIVES[];

  static constexpr int NOUNS_SIZE = 49;
  static const char* NOUNS[];

private:
  static thread_local std::mt19937 RANDOM_GENERATOR;
public:

  static oatpp::String random();

};

#endif // Nickname_hpp
