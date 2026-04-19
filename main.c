#include <stdio.h>

#include "analizor-lexical/lexer.h"
#include "analizor-lexical/utils.h"
#include "analizor-sintactic/parser.h"

#define LEXER_OUTPUT_FILEPATH "lexer_out.txt"

Token *analizorLexical(char *testFilePath) {
    Token *tokenList = NULL;

    char *fileContent = getFileContent(testFilePath);

    tokenList = tokenize(fileContent);

    printTokensToFile(LEXER_OUTPUT_FILEPATH, tokenList);

    return tokenList;
}

void analizorSintactic(Token *tokenList) {
    parse(tokenList);
}

int main(int argc, char *argv[]) {
    char *testFilePath = argv[1];

    Token *tokenList = analizorLexical(testFilePath);
    analizorSintactic(tokenList);

    return 0;
}