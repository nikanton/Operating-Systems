#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CHAR_SIZE sizeof(char)

void strings(FILE* file) {
  int letter;
  char* currentString = NULL;
  size_t currentStringSize = 0;
  do {
    letter = fgetc(file);
    if(letter == '\n' || letter == EOF){
      if(currentStringSize >= 4) {
        currentString = realloc(currentString, ++currentStringSize * CHAR_SIZE);
        currentString[currentStringSize - 1] = '\0';
        printf("%s\n", currentString);
      }
      free(currentString);
      currentString = NULL;
      currentStringSize = 0;
    }
    else {
      if(letter == 9 || letter == ' ' || (letter> 32 && letter < 128)) {
        currentString = realloc(currentString, ++currentStringSize * CHAR_SIZE);
        currentString[currentStringSize - 1] = letter;
      }
    }
  } while(letter != EOF);
}

int main(int argc, char** argv) {
  if (argc == 1)
    strings(stdin);
  else if (argc == 2) {
    FILE *File = fopen(argv[1], "r");
    if (File != NULL) {
      strings(File);
      fclose(File);
    }
    else
      printf("Can not open.\n");
  }
  else {
    for (size_t i = 1; i < argc; ++i) {
      printf("%s:\n",  argv[i]);
      FILE* currentFile = fopen(argv[i], "r");
      if (currentFile != NULL) {
        strings(currentFile);
        fclose(currentFile);
      }
      else
        printf("Can not open.\n");
    }
  }
}
