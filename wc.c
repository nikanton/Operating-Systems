#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int currentFileStrings;
int currentFileWords;
int currentFileSymbols;

void wc(FILE* file) {
    currentFileStrings = 0;
    currentFileWords = 0;
    currentFileSymbols = 0;
    
    int currentSymbol;
    int currentWordLength = 0;
    
    while((currentSymbol = fgetc(file)) != EOF) {
        ++currentFileSymbols;
        if(isspace(currentSymbol) && currentWordLength)
            ++currentFileWords;
        if(currentSymbol == '\n')
            ++currentFileStrings;
        if(!isspace(currentSymbol))
            ++currentWordLength;
        else
            currentWordLength = 0;
    }

    if(currentWordLength)
        ++currentFileWords;
}

int main(int argc, char** argv) {
    int totalStrings = 0;
    int totalWords = 0;
    int totalSymbols = 0;
    
    if(argc == 1) {
        wc(stdin);
        printf("%d %d %d\n", currentFileStrings, currentFileWords, currentFileSymbols);
    }

    for(int i = 1; i < argc; ++i) {
        FILE* file = fopen(argv[i], "r");
	wc(file);
	printf("%d %d %d %s\n", currentFileStrings, currentFileWords, currentFileSymbols, argv[i]);
	totalStrings += currentFileStrings;
	totalWords += currentFileWords;
	totalSymbols += currentFileSymbols;
    }
    
    if(argc > 2)
        printf("%d %d %d %s\n", totalStrings, totalWords, totalSymbols, "total");        
    
    return 0;
}
