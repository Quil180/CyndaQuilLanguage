#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

enum class tokenType { _return, int_lit, end_line };

struct Token {
  tokenType type;
  std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string &str) {
  std::vector<Token> tokens;

  std::string buffer;
  for (int i = 0; i < str.length(); i++) {
    char c = str.at(i);
    if (std::isalpha(c)) {
      buffer.push_back(c);
      i++;
      while (std::isalnum(str.at(i))) {
        buffer.push_back(str.at(i));
        i++;
      }
      i--;
      if (buffer == "gift") {
        std::cout << "We found a return statement..." << std::endl;
        tokens.push_back({.type = tokenType::_return});
        buffer.clear();
        continue;
      } else {
        std::cerr << "You messed up!" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (std::isdigit(c)) {
      buffer.push_back(c);
      i++;
      while (std::isdigit(str.at(i))) {
        buffer.push_back(str.at(i));
        i++;
      }
      i--;
      std::cout << "We found a number..." << std::endl;
      tokens.push_back({.type = tokenType::int_lit, .value = buffer});
      buffer.clear();
    } else if (c == '.') {
      std::cout << "We found an end line statement" << std::endl;
      tokens.push_back({.type = tokenType::end_line});
    } else if (std::isspace(c)) {
      continue;
    } else {
      std::cerr << "You messed up!" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  return tokens;
}

std::string convertToASM(std::vector<Token> &tokens) {
  std::stringstream output;
  output << "global _start\nstart:\n"; // initializing the start of the
                                       // stringstream
  std::cout << "Tokens size = " << tokens.size() << std::endl;
  for (int i = 0; i < tokens.size(); i++) {
    const Token &token = tokens.at(i);
    if (token.type == tokenType::_return) {
      std::cout << "conversion found a return token" << std::endl;
      if (i + 1 < tokens.size() &&
          tokens.at(i + 1).type == tokenType::int_lit) {
        std::cout << "conversion found a integer token" << std::endl;
        if (i + 2 < tokens.size() &&
            tokens.at(i + 2).type == tokenType::end_line) {
          std::cout << "conversion found a end line token" << std::endl;
          // we have a valid return statement.
          output << "  mov rax, 60\n";
          output << "  mov rdi, " << tokens.at(i + 1).value.value() << "\n";
          output << "  syscall\n";
        }
      }
    }
  }
  return output.str();
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    // we were not inputted a file to compile.
    std::cerr << "Incorrect usage... Correct usage is..." << std::endl;
    std::cerr << "ember <input.cq>" << std::endl;
    return EXIT_FAILURE;
  }
  // from this point on, we have been given one file to compile.

  std::fstream input(argv[1], std::ios::in); // declaring where to read from.
  std::stringstream content_stream;          // making the stream to read into
  content_stream << input.rdbuf();           // reading the file input.
  input.close();                             // closing the file

  // converting the stream into a string.
  std::string contents = content_stream.str();

  std::vector<Token> tokens = tokenize(contents);

  std::cout << convertToASM(tokens) << std::endl;

  return EXIT_SUCCESS;
}
