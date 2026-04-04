#include <stdio.h>

#include "analizor-lexical/lexer.h"
#include "analizor-lexical/utils.h"

#define TEST_FILEPATH "tests/testfile.c"
#define LEXER_OUTPUT_FILEPATH "lexer_out.txt"

void analizorLexical() {
    Token *tokenList = NULL;
    // Token *lastToken = NULL;

    char *fileContent = getFileContent(TEST_FILEPATH);

    tokenList = tokenize(fileContent);

    printTokensToFile(LEXER_OUTPUT_FILEPATH, tokenList);
}

int main() {
    analizorLexical();
    return 0;
}