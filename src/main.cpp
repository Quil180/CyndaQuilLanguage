#include "tokener.hpp"
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

std::string convertToASM(std::vector<Token> &tokens) {
  std::stringstream output;
  output << "global _start\n_start:\n"; // initializing the start of the
                                        // stringstream
  for (int i = 0; i < tokens.size(); i++) {
    const Token &token = tokens.at(i);
    if (token.type == tokenType::run) {
      if (i + 1 < tokens.size() &&
          tokens.at(i + 1).type == tokenType::int_lit) {
        if (i + 2 < tokens.size() &&
            tokens.at(i + 2).type == tokenType::end_line) {
          // we have a valid return statement.
          output << "    mov rax, 60\n";
          output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
          output << "    syscall\n";
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

  Tokenizer tokenizer(std::move(contents));
  std::vector<Token> tokens = tokenizer.tokenize();

  std::fstream file("ember.asm", std::ios::out);
  file << convertToASM(tokens);
  file.close();

  // linking the assembly
  system("nasm -felf64 ember.asm");
  // making the object file so we can run it at will o7.
  system("ld -o out ember.o");

  return EXIT_SUCCESS;
}
