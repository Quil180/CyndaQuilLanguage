#pragma once

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

enum class tokenType {
  run,
  int_lit,
  end_line,
  open_paren,
  close_paren,
  ident,
  _catch,
  as,
  plus,
  star,
  sub,
  div
};

std::optional<int> binary_precedence(tokenType type) {
  switch (type) {
  case tokenType::plus:
    return 0;
  case tokenType::sub:
    return 0;
  case tokenType::star:
    return 1;
  case tokenType::div:
    return 1;
  default:
    return {};
  }
}

struct Token {
  tokenType type;
  std::optional<std::string> value{};
};

class Tokenizer {
public:
  inline explicit Tokenizer(const std::string source)
      : mem_source(std::move(source)) {}

  inline std::vector<Token> tokenize() {
    std::vector<Token> tokens;
    std::string buffer;

    while (peek().has_value()) { // while peak has a value returned
      // the next index is guaranteed to exist.
      if (std::isalpha(peek().value())) {
        // push the at the current index into the buffer.
        buffer.push_back(consume());
        while (peek().has_value() && std::isalnum(peek().value())) {
          // keep on consuming until we finish "eating" all the numbers/letters.
          buffer.push_back(consume());
        }
        // checking if the characters found is a keyword.
        if (buffer == "run") {
          // we found a return statement.
          tokens.push_back({.type = tokenType::run});
          buffer.clear();
          continue;
        } else if (buffer == "catch") {
          tokens.push_back({.type = tokenType::_catch});
          buffer.clear();
          continue;
        } else if (buffer == "as") {
          tokens.push_back({.type = tokenType::as});
          buffer.clear();
          continue;
        } else {
          // no valid special keyword was found therefore its an identifier.
          tokens.push_back({.type = tokenType::ident, .value = buffer});
          buffer.clear();
          continue;
        }
      } else if (std::isdigit(peek().value())) {
        buffer.push_back(consume());
        while (peek().has_value() && std::isdigit(peek().value())) {
          buffer.push_back(consume());
        }
        tokens.push_back({.type = tokenType::int_lit, .value = buffer});
        buffer.clear();
        continue;
      } else if (peek().value() == '(') {
        consume();
        tokens.push_back({.type = tokenType::open_paren});
      } else if (peek().value() == ')') {
        consume();
        tokens.push_back({.type = tokenType::close_paren});
        continue;
        tokens.push_back({.type = tokenType::close_paren});
      } else if (peek().value() == '~') {
        consume();
        tokens.push_back({.type = tokenType::end_line});
        continue;
      } else if (peek().value() == '+') {
        consume();
        tokens.push_back({.type = tokenType::plus});
        continue;
      } else if (peek().value() == '*') {
        consume();
        tokens.push_back({.type = tokenType::star});
        continue;
      } else if (peek().value() == '/') {
        consume();
        tokens.push_back({.type = tokenType::div});
        continue;
      } else if (peek().value() == '-') {
        consume();
        tokens.push_back({.type = tokenType::sub});
        continue;
      } else if (std::isspace(peek().value())) {
        consume();
        continue;
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
  size_t mem_index = 0;         // current index the tokenizer is at.

  // methods.
  // nodiscard will tell us if there is no return value (aka something went
  // wrong.)
  [[nodiscard]] inline std::optional<char>
  peek(int offset = 0) const { // const as it shouldnt edit anything.
    if (mem_index + offset >= mem_source.length()) {
      return {};
    } else {
      return mem_source.at(mem_index +
                           offset); // return the character at that index.
    }
  }

  inline char consume() { return mem_source.at(mem_index++); }
};
