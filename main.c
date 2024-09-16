#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TokenType {
  _return,
  integer_literal,
  end,
};

typedef struct Token {
  enum TokenType type;
  char *value;
} Token;

Token *tokenizer(const char *string) {
  int buffersize = 1;
  char *buffer = (char *)malloc(buffersize);
  int amounttokens = 1;
  Token *tokens = (Token *)malloc(amounttokens * sizeof(Token));
  tokens[amounttokens - 1].type = 0;
  tokens[amounttokens - 1].value = NULL;

  for (int i = 0; i < strlen(string); i++) {
    char c = string[i];
    // getting each character in the cyndaquil file
    if (isalpha(c)) {
      // the given char is a alpha
      // therefore we will read it into a buffer (malloced array)
      // the following is me reading into the buffer
      buffersize += 1;
      buffer = (char *)realloc(buffer, buffersize);
      buffer[buffersize - 1] = c;
      // then going to the next value
      i++;
      while (isalnum(string[i])) {
        // this will only occur if the token continues (until a non-alphanumeric
        // thing is found)
        buffersize += 1;
        buffer = (char *)realloc(buffer, buffersize);
        buffer[buffersize - 1] = string[i];
        i++;
      }
      // done to reset the place of the buffer
      i--;

      // now we can check if this is a cool keyword!!
      if (strcmp("@return", buffer) == 0) {
        // a return token has been found
        // this token was never initialized
        tokens[amounttokens - 1].type = _return;
        amounttokens += 1;
        tokens = (Token *)realloc(tokens, amounttokens * sizeof(Token));
        // now we must find the value of the return

        // clearing the buffer so we can continue on
        buffersize = 1;
        buffer = (char *)realloc(buffer, buffersize);
        buffer[buffersize - 1] = 0;
        continue;
      } else {
        // the return token has not been found
        printf("The only token possible of return wasn't found");
        return NULL; // an error has occured
      }
    } else if (isdigit(c)) {
      // we now have digits
      buffersize += 1;
      buffer = (char *)realloc(buffer, buffersize);
      buffer[buffersize - 1] = c;
      i++;

      while (isdigit(c)) {
        // there is still digits
        buffersize += 1;
        buffer = (char *)realloc(buffer, buffersize);
        buffer[buffersize - 1] = c;
        i++;
      }
      tokens[amounttokens - 1].type = integer_literal;
      // allocate the memory for the stupid value of the token
      tokens[amounttokens - 1].value = buffer;
      tokens = (Token *)realloc(tokens, amounttokens * sizeof(Token));
      continue;
    } else if (isspace(c)) {
      // for whitespace, we ignore it
      continue;
    }
  }
  free(buffer);

  return tokens;
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Incorrect Usage...\n");
    printf("fireball <input.cq>\n");
    return 1;
  }

  // opening the file for reading only (not writing)
  FILE *cyndaquil = fopen(argv[1], "r");

  fseek(cyndaquil, 0, SEEK_END);    // get the end of the file
  long filesize = ftell(cyndaquil); // get the filesize
  fseek(cyndaquil, 0, SEEK_SET);    // get back to the beginning of the file

  char *filetext = (char *)malloc(filesize + 1);
  fread(filetext, filesize, 1,
        cyndaquil);  // reading the text into the filetext variable
  fclose(cyndaquil); // closing the file as it is now unneeded

  // we can now continue with the compiler :)

  // creating a really bad lexilizer
  Token *tokens = tokenizer(filetext);

  printf("%s\n", filetext);

  // freeing the memory of the filetext
  free(tokens);
  free(filetext);

  return 0;
}
