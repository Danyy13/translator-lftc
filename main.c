#include <stdio.h>

#include "analizor-lexical/lexer.h"
#include "analizor-lexical/utils.h"

#define LEXER_OUTPUT_FILEPATH "lexer_out.txt"

void analizorLexical(char *testFilePath) {
    Token *tokenList = NULL;
    // Token *lastToken = NULL;

    char *fileContent = getFileContent(testFilePath);

    tokenList = tokenize(fileContent);

    printTokensToFile(LEXER_OUTPUT_FILEPATH, tokenList);
}

int main(int argc, char *argv[]) {
    char *testFilePath = argv[1];
    
    analizorLexical(testFilePath);
    return 0;
}