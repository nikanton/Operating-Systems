#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void sed(char* textForReplacement, char* textForSubstitution, FILE* file) {
  int letter;
  while ((letter = fgetc(file)) != EOF) {
    if (letter != textForReplacement[0])
      printf("%c", letter);
    else {
      int sameLetters = 1;
      while(sameLetters < strlen(textForReplacement) &&
          (letter = fgetc(file)) == textForReplacement[sameLetters])
        ++sameLetters;
      if(sameLetters == strlen(textForReplacement))
        printf("%s", textForSubstitution);
      else {
        for (int i = 0; i < sameLetters; ++i)
          printf("%c", textForReplacement[i]);
        if(letter == EOF)
          break;
        else
          printf("%c", letter);
      }
    }
  }
  printf("\n");
}

int main(int argc, char** argv) {
  if (argc < 3)
    printf("Too little arguments.\n");
  else if (argc == 3)
    sed(argv[1], argv[2], stdin);
  else {
    for (size_t i = 3; i < argc; ++i) {
      FILE* currentFile = fopen(argv[i], "r");
      if (currentFile != NULL) {
        sed(argv[1], argv[2], currentFile);
        fclose(currentFile);
      }
      else
        printf("Can not open %s.\n", argv[i]);
    }
  }
}