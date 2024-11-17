#pragma once

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

enum class tokenType { run, int_lit, end_line };

struct Token {
  tokenType type;
  std::optional<std::string> value;
};

class Tokenizer {
public:
  inline explicit Tokenizer(const std::string source) : mem_source(std::move(source)) {}
  inline std::vector<Token> tokenize() {
    std::vector<Token> tokens;
    std::string buffer;

    while (peak().has_value()) { // while peak has a value returned
      // the next index is guaranteed to exist.
      if (std::isalpha(peak().value())) {
        // push the at the current index into the buffer.
        buffer.push_back(consume());
        while (peak().has_value() && std::isalnum(peak().value())) {
          // keep on consuming until we finish "eating" all the numbers/letters.
          buffer.push_back(consume());
        }
        // checking if the characters found is a keyword.
        if (buffer == "run") {
          // we found a return statement.
          tokens.push_back({.type = tokenType::run});
          buffer.clear();
          continue;
        } else {
          // no valid special keyword was found
          std::cerr << "No special keyword was found..." << std::endl;
          exit(EXIT_FAILURE);
        }
      } else if (std::isdigit(peak().value())) {
        buffer.push_back(consume());
        while (peak().has_value() && std::isdigit(peak().value())) {
          buffer.push_back(consume());
        }
        tokens.push_back({.type = tokenType::int_lit, .value = buffer});
        buffer.clear();
        continue;
      } else if (peak().value() == '~') {
        consume();
        tokens.push_back({.type = tokenType::end_line});
        continue;
      } else if (std::isspace(peak().value())) {
        consume();
      } else {
        // no tokentype could be assigned.
        std::cerr << "No token type could be assigned..." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    mem_index = 0; // resetting for if we want to tokenize again.
    return tokens;
  }

private:
  // variables.
  const std::string mem_source; // the source code given to the tokenizer.
  int mem_index = 0;                // current index the tokenizer is at.

  // methods.
  // nodiscard will tell us if there is no return value (aka something went
  // wrong.)
  [[nodiscard]] std::optional<char>
  peak(int ahead = 1) const { // const as it shouldnt edit anything.
    if (mem_index + ahead > mem_source.length()) {
      return {};
    } else {
      return mem_source.at(mem_index); // return the character at that index.
    }
  }

  char consume() { return mem_source.at(mem_index++); }
};
