#include "asm_generator.hpp"
#include <fstream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    // we were not inputted a file to compile.
    std::cerr << "Incorrect usage... Correct usage is..." << std::endl;
    std::cerr << "ember <input.cq>" << std::endl;
    return EXIT_FAILURE;
  }
  // from this point on, we have been given one file to compile.

  std::fstream input(argv[1], std::ios::in);      // declaring where to read from.
  std::stringstream content_stream;          // making the stream to read into
  content_stream << input.rdbuf();           // reading the file input.
  input.close();                             // closing the file

  // converting the stream into a string.
  std::string contents = content_stream.str();

  // tokenizing the stringstream from the input file.
  Tokenizer tokenizer(std::move(contents));
  std::vector<Token> tokens = tokenizer.tokenize();

  // 
  Parser parser(std::move(tokens));
  std::optional<nodeRun> parsedTree = parser.parse();

  if (!parsedTree.has_value()) {
    std::cerr << "No run statement found..." << std::endl;
    exit(EXIT_FAILURE);
  }

  ASMGenerator generator(parsedTree.value());

  std::fstream file("ember.asm", std::ios::out);
  file << generator.generate_asm();
  file.close();

  // linking the assembly
  system("nasm -felf64 ember.asm");
  // making the object file so we can run it at will o7.
  system("ld -o out ember.o");

  return EXIT_SUCCESS;
}
